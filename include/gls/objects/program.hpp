/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/object.hpp>
#include <gls/objects/shader.hpp>
#include <initializer_list>
#include <vector>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <cassert>

#if !defined( NDEBUG )
	#if !defined( GLS_ERROR_STREAM )
		#include <iostream>
		#define GLS_ERROR_STREAM std::cerr
	#endif
#endif

namespace gls {

namespace priv {
	void create_program( GLsizei, GLuint* name ) { *name = glCreateProgram(); }
	void delete_program( GLsizei, const GLuint* name ) { glDeleteProgram( *name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL program object
///
////////////////////////////////////////////////////////////////////////////////
class program {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///
	//////////////////////////////////////////////////////////////////////////////
	program() {
		check_gl_error( glGetIntegerv( GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_uniform_buffer_bindings ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this program
	///
	/// \return OpenGL name of this program
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Link a set of compiled shaders into this program
	///
	/// After successfully compiling a set of gls::shader objects, call this
	/// method to link them into this program. They will be attached, linked and
	/// detached thereafter. Checking the produced information log with
	/// get_info_log() can help to solve errors that occur or warnings the driver
	/// might have produced.
	///
	/// After linking successfully, all attribute, uniform and block information
	/// will be extracted from the program.
	///
	/// Uniform blocks will automatically be bound to a unique binding point which
	/// can be queried with get_uniform_block_binding(). If there are more blocks
	/// present in a program than there are binding points, an assertion will
	/// terminate the application.
	///
	/// \param shaders Variable collection of shaders to link to this program
	/// \return true if linking was successful, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename... Shaders>
	bool link( Shaders&&... shaders ) {
		attach_all( shaders... );

		check_gl_error( glLinkProgram( name() ) );

		detach_all( shaders... );

		auto link_status = GL_FALSE;
		check_gl_error( glGetProgramiv( name(), GL_LINK_STATUS, &link_status ) );

#if !defined( NDEBUG )
		auto info_log = get_info_log();

		if( !info_log.empty() ) {
			GLS_ERROR_STREAM << std::move( info_log );
		}
#endif

		auto result = ( link_status == GL_TRUE );

		if( result ) {
			introspect();
		}

		return result;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the link information log
	///
	/// After linking, whether successful or not, an information log might be
	/// available. Check this information log for warnings after successful
	/// linking, or errors after failed linking. The information log may be empty
	/// if linking was successful.
	///
	/// \return String containing the information log
	///
	//////////////////////////////////////////////////////////////////////////////
	std::string get_info_log() const {
		auto info_log_length = GLint();
		check_gl_error( glGetProgramiv( name(), GL_INFO_LOG_LENGTH, &info_log_length ) );

		if( !info_log_length ) {
			return std::string();
		}

		auto info_log = std::vector<GLchar>( static_cast<std::size_t>( info_log_length ), 0 );
		check_gl_error( glGetProgramInfoLog( name(), info_log_length, nullptr, info_log.data() ) );

		return std::string( info_log.data() );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Use this program
	///
	/// Set the program as the currently used one. This replaces any previously
	/// current program.
	///
	//////////////////////////////////////////////////////////////////////////////
	void use() {
		check_gl_error( glUseProgram( name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief No longer use a program
	///
	/// Clear the currently used program.
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unuse() {
		check_gl_error( glUseProgram( 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the location of an attribute
	///
	/// After linking, you can query the program for the location of an attribute.
	/// You can use this value to set the source that the attribute should pull
	/// data from.
	///
	/// \param attribute_name Name of the attribute
	/// \return Location of the attribute, or -1 if the attribute does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLint get_attribute_location( const std::string& attribute_name ) const {
		const auto iter = m_attribute_map.find( attribute_name );

		if( iter == std::end( m_attribute_map ) ) {
			return -1;
		}

		return std::get<0>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the type of an attribute
	///
	/// After linking, you can query the program for the type of an attribute.
	///
	/// \param attribute_name Name of the attribute
	/// \return Type of the attribute, or 0 if the attribute does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLenum get_attribute_type( const std::string& attribute_name ) const {
		const auto iter = m_attribute_map.find( attribute_name );

		if( iter == std::end( m_attribute_map ) ) {
			return 0u;
		}

		return std::get<1>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of an attribute
	///
	/// After linking, you can query the program for the size of an attribute.
	///
	/// \param attribute_name Name of the attribute
	/// \return Size of the attribute, or 0 if the attribute does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLint get_attribute_size( const std::string& attribute_name ) const {
		const auto iter = m_attribute_map.find( attribute_name );

		if( iter == std::end( m_attribute_map ) ) {
			return 0;
		}

		return std::get<2>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the location of a uniform
	///
	/// After linking, you can query the program for the location of a uniform.
	/// You can use this location to set the value that the uniform should be set
	/// to when the program is run.
	///
	/// \param uniform_name Name of the uniform
	/// \return Location of the uniform, or -1 if the uniform does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLint get_uniform_location( const std::string& uniform_name ) const {
		const auto iter = m_uniform_map.find( uniform_name );

		if( iter == std::end( m_uniform_map ) ) {
			return -1;
		}

		return std::get<0>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the type of a uniform
	///
	/// After linking, you can query the program for the type of a uniform.
	///
	/// \param uniform_name Name of the uniform
	/// \return Type of the uniform, or 0 if the uniform does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLenum get_uniform_type( const std::string& uniform_name ) const {
		const auto iter = m_uniform_map.find( uniform_name );

		if( iter == std::end( m_uniform_map ) ) {
			return 0u;
		}

		return std::get<1>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of a uniform
	///
	/// After linking, you can query the program for the size of a uniform.
	///
	/// \param uniform_name Name of the uniform
	/// \return Size of the uniform, or 0 if the uniform does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLint get_uniform_size( const std::string& uniform_name ) const {
		const auto iter = m_uniform_map.find( uniform_name );

		if( iter == std::end( m_uniform_map ) ) {
			return 0;
		}

		return std::get<2>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the binding point of a uniform block
	///
	/// After linking, all uniform blocks are bound to unique binding points. You
	/// can query the program for the binding point mapped to each block using
	/// this method. You can use the binding point to set the source buffer of the
	/// block that data should originate from when the program is run.
	///
	/// \param uniform_block_name Name of the uniform block
	/// \return Binding point mapped to the uniform block, or no_block_binding() if the block does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint get_uniform_block_binding( const std::string& uniform_block_name ) const {
		const auto iter = m_uniform_block_map.find( uniform_block_name );

		if( iter == std::end( m_uniform_block_map ) ) {
			return no_block_binding();
		}

		return std::get<0>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of a uniform block
	///
	/// Use this to determine the size requirements of a uniform block with
	/// binding it to its source buffer.
	///
	/// \param uniform_block_name Name of the uniform block
	/// \return Size of the uniform block in bytes, or 0 if the block does not exist
	///
	//////////////////////////////////////////////////////////////////////////////
	GLint get_uniform_block_size( const std::string& uniform_block_name ) const {
		const auto iter = m_uniform_block_map.find( uniform_block_name );

		if( iter == std::end( m_uniform_block_map ) ) {
			return 0;
		}

		return std::get<1>( iter->second );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Value to indicate a non-existant block binding
	///
	/// \return Value that corresponds to a non-existant block binding
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint no_block_binding() const {
		return static_cast<GLuint>( m_max_uniform_buffer_bindings );
	}

	/// @cond
	template<typename T>
	void uniform( const std::string& uniform_name, T v0 ) = delete;
	/// @endcond

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 1-component uniform value
	///
	/// Set a 1-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLfloat v0 ) { use(); check_gl_error( glUniform1f( get_uniform_location( uniform_name ), v0 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 1-component uniform value
	///
	/// Set a 1-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLint v0 ) { use(); check_gl_error( glUniform1i( get_uniform_location( uniform_name ), v0 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 1-component uniform value
	///
	/// Set a 1-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLuint v0 ) { use(); check_gl_error( glUniform1ui( get_uniform_location( uniform_name ), v0 ) ); unuse(); };

	/// @cond
	template<typename T>
	void uniform( const std::string& uniform_name, T v0, T v1 ) = delete;
	/// @endcond

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 2-component uniform value
	///
	/// Set a 2-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLfloat v0, GLfloat v1 ) { use(); check_gl_error( glUniform2f( get_uniform_location( uniform_name ), v0, v1 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 2-component uniform value
	///
	/// Set a 2-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLint v0, GLint v1 ) { use(); check_gl_error( glUniform2i( get_uniform_location( uniform_name ), v0, v1 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 2-component uniform value
	///
	/// Set a 2-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLuint v0, GLuint v1 ) { use(); check_gl_error( glUniform2ui( get_uniform_location( uniform_name ), v0, v1 ) ); unuse(); };

	/// @cond
	template<typename T>
	void uniform( const std::string& uniform_name, T v0, T v1, T v2 ) = delete;
	/// @endcond

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 3-component uniform value
	///
	/// Set a 3-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	/// \param v2 Third component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLfloat v0, GLfloat v1, GLfloat v2 ) { use(); check_gl_error( glUniform3f( get_uniform_location( uniform_name ), v0, v1, v2 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 3-component uniform value
	///
	/// Set a 3-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	/// \param v2 Third component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLint v0, GLint v1, GLint v2 ) { use(); check_gl_error( glUniform3i( get_uniform_location( uniform_name ), v0, v1, v2 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 3-component uniform value
	///
	/// Set a 3-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	/// \param v2 Third component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLuint v0, GLuint v1, GLuint v2 ) { use(); check_gl_error( glUniform3ui( get_uniform_location( uniform_name ), v0, v1, v2 ) ); unuse(); };

	/// @cond
	template<typename T>
	void uniform( const std::string& uniform_name, T v0, T v1, T v2, T v3 ) = delete;
	/// @endcond

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 4-component uniform value
	///
	/// Set a 4-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	/// \param v2 Third component
	/// \param v3 Fourth component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) { use(); check_gl_error( glUniform4f( get_uniform_location( uniform_name ), v0, v1, v2, v3 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 4-component uniform value
	///
	/// Set a 4-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	/// \param v2 Third component
	/// \param v3 Fourth component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLint v0, GLint v1, GLint v2, GLint v3 ) { use(); check_gl_error( glUniform4i( get_uniform_location( uniform_name ), v0, v1, v2, v3 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a 4-component uniform value
	///
	/// Set a 4-component uniform with the given name to the given value.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param v0 First component
	/// \param v1 Second component
	/// \param v2 Third component
	/// \param v3 Fourth component
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform( const std::string& uniform_name, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) { use(); check_gl_error( glUniform4ui( get_uniform_location( uniform_name ), v0, v1, v2, v3 ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 1-component uniform values
	///
	/// Set an array of 1-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform1( const std::string& uniform_name, GLsizei count, const GLfloat* value ) { use(); check_gl_error( glUniform1fv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 1-component uniform values
	///
	/// Set an array of 1-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform1( const std::string& uniform_name, GLsizei count, const GLint* value ) { use(); check_gl_error( glUniform1iv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 1-component uniform values
	///
	/// Set an array of 1-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform1( const std::string& uniform_name, GLsizei count, const GLuint* value ) { use(); check_gl_error( glUniform1uiv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 2-component uniform values
	///
	/// Set an array of 2-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform2( const std::string& uniform_name, GLsizei count, const GLfloat* value ) { use(); check_gl_error( glUniform2fv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 2-component uniform values
	///
	/// Set an array of 2-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform2( const std::string& uniform_name, GLsizei count, const GLint* value ) { use(); check_gl_error( glUniform2iv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 2-component uniform values
	///
	/// Set an array of 2-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform2( const std::string& uniform_name, GLsizei count, const GLuint* value ) { use(); check_gl_error( glUniform2uiv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 3-component uniform values
	///
	/// Set an array of 3-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform3( const std::string& uniform_name, GLsizei count, const GLfloat* value ) { use(); check_gl_error( glUniform3fv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 3-component uniform values
	///
	/// Set an array of 3-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform3( const std::string& uniform_name, GLsizei count, const GLint* value ) { use(); check_gl_error( glUniform3iv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 3-component uniform values
	///
	/// Set an array of 3-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform3( const std::string& uniform_name, GLsizei count, const GLuint* value ) { use(); check_gl_error( glUniform3uiv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 4-component uniform values
	///
	/// Set an array of 4-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform4( const std::string& uniform_name, GLsizei count, const GLfloat* value ) { use(); check_gl_error( glUniform4fv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 4-component uniform values
	///
	/// Set an array of 4-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform4( const std::string& uniform_name, GLsizei count, const GLint* value ) { use(); check_gl_error( glUniform4iv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 4-component uniform values
	///
	/// Set an array of 4-component uniforms with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform4( const std::string& uniform_name, GLsizei count, const GLuint* value ) { use(); check_gl_error( glUniform4uiv( get_uniform_location( uniform_name ), count, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 2x2 uniform matrix values
	///
	/// Set an array of 2x2 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix2( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix2fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 3x3 uniform matrix values
	///
	/// Set an array of 3x3 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix3( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix3fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 4x4 uniform matrix values
	///
	/// Set an array of 4x4 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix4( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix4fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 2x3 uniform matrix values
	///
	/// Set an array of 2x3 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix2x3( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix2x3fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 3x2 uniform matrix values
	///
	/// Set an array of 3x2 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix3x2( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix3x2fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 2x4 uniform matrix values
	///
	/// Set an array of 2x4 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix2x4( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix2x4fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 4x2 uniform matrix values
	///
	/// Set an array of 4x2 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix4x2( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix4x2fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 3x4 uniform matrix values
	///
	/// Set an array of 3x4 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix3x4( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix3x4fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set an array of 4x3 uniform matrix values
	///
	/// Set an array of 4x3 uniform matrices with the given name to the given
	/// values.
	///
	/// \param uniform_name Name of the uniform to set
	/// \param count Number of array elements
	/// \param transpose GL_TRUE to transpose before loading, GL_FALSE not to
	/// \param value Array of values
	///
	//////////////////////////////////////////////////////////////////////////////
	void uniform_matrix4x3( const std::string& uniform_name, GLsizei count, GLboolean transpose, const GLfloat* value ) { use(); check_gl_error( glUniformMatrix4x3fv( get_uniform_location( uniform_name ), count, transpose, value ) ); unuse(); };

private:
	/// @cond
	struct pass {
    template<typename ...T>
    pass( T... ) {
    }
  };
  /// @endcond

	template<typename... Args>
	void attach_all( Args&&... args ) {
		pass( ( attach( args ), 1 )... );
	}

	template<GLenum ShaderType>
	void attach( const shader<ShaderType>& shader_object ) {
		check_gl_error( glAttachShader( name(), shader_object.name() ) );
	}

	template<typename... Args>
	void detach_all( Args&&... args ) {
		pass( ( detach( args ), 1 )... );
	}

	template<GLenum ShaderType>
	void detach( const shader<ShaderType>& shader_object ) {
		check_gl_error( glDetachShader( name(), shader_object.name() ) );
	}

	void introspect() {
		// Attributes
		{
			m_attribute_map.clear();

			auto active_attributes = GLint();
			check_gl_error( glGetProgramiv( name(), GL_ACTIVE_ATTRIBUTES, &active_attributes ) );

			auto active_attribute_max_length = GLint();
			check_gl_error( glGetProgramiv( name(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &active_attribute_max_length ) );

			auto attribute_name = std::vector<GLchar>( static_cast<std::size_t>( active_attribute_max_length ), 0 );

			for( GLuint index = 0; index < static_cast<GLuint>( active_attributes ); ++index ) {
				auto attribute_type = GLenum();
				auto attribute_size = GLint();

				check_gl_error( glGetActiveAttrib( name(), index, active_attribute_max_length, nullptr, &attribute_size, &attribute_type, attribute_name.data() ) );

				auto attribute_location = GLint();

				attribute_location = check_gl_error( glGetAttribLocation( name(), attribute_name.data() ) );

				assert( attribute_location >= 0 );

				m_attribute_map.emplace( std::string( attribute_name.data() ), std::make_tuple( attribute_location, attribute_type, attribute_size ) );
			}
		}

		// Uniforms
		{
			m_uniform_map.clear();

			auto active_uniforms = GLint();
			check_gl_error( glGetProgramiv( name(), GL_ACTIVE_UNIFORMS, &active_uniforms ) );

			auto active_uniform_max_length = GLint();
			check_gl_error( glGetProgramiv( name(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &active_uniform_max_length ) );

			auto uniform_name = std::vector<GLchar>( static_cast<std::size_t>( active_uniform_max_length ), 0 );

			for( GLuint index = 0; index < static_cast<GLuint>( active_uniforms ); ++index ) {
				auto uniform_type = GLint();
				auto uniform_size = GLint();

				check_gl_error( glGetActiveUniformName( name(), index, active_uniform_max_length, nullptr, uniform_name.data() ) );
				check_gl_error( glGetActiveUniformsiv( name(), 1, &index, GL_UNIFORM_TYPE, &uniform_type ) );
				check_gl_error( glGetActiveUniformsiv( name(), 1, &index, GL_UNIFORM_SIZE, &uniform_size ) );

				auto uniform_location = GLint();

				uniform_location = check_gl_error( glGetUniformLocation( name(), uniform_name.data() ) );

				assert( uniform_location >= 0 );

				m_uniform_map.emplace( std::string( uniform_name.data() ), std::make_tuple( uniform_location, static_cast<GLenum>( uniform_type ), uniform_size ) );
			}
		}

		// Uniform blocks
		{
			m_uniform_block_map.clear();

			auto active_uniform_blocks = GLint();
			check_gl_error( glGetProgramiv( name(), GL_ACTIVE_UNIFORM_BLOCKS, &active_uniform_blocks ) );

			auto active_uniform_block_max_name_length = GLint();
			check_gl_error( glGetProgramiv( name(), GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &active_uniform_block_max_name_length ) );

			auto uniform_block_name = std::vector<GLchar>( static_cast<std::size_t>( active_uniform_block_max_name_length ), 0 );

#if !defined( NDEBUG )
			if( active_uniform_blocks > m_max_uniform_buffer_bindings ) {
				GLS_ERROR_STREAM << "Error: Couldn't bind all uniform blocks." << std::endl;
				GLS_ERROR_STREAM << "Active blocks: " << active_uniform_blocks << " Max bindings: " << m_max_uniform_buffer_bindings << std::endl;

				active_uniform_blocks = m_max_uniform_buffer_bindings;
			}
#endif

			for( GLuint index = 0; index < static_cast<GLuint>( active_uniform_blocks ); ++index ) {
				auto uniform_block_size = GLint();

				check_gl_error( glGetActiveUniformBlockName( name(), index, active_uniform_block_max_name_length, nullptr, uniform_block_name.data() ) );
				check_gl_error( glGetActiveUniformBlockiv( name(), index, GL_UNIFORM_BLOCK_DATA_SIZE, &uniform_block_size ) );

				m_uniform_block_map.emplace( std::string( uniform_block_name.data() ), std::make_tuple( index, uniform_block_size ) );
			}
		}
	}

	object<priv::create_program, priv::delete_program> m_object;
	std::unordered_map<std::string, std::tuple<GLint, GLenum, GLint>> m_attribute_map;
	std::unordered_map<std::string, std::tuple<GLint, GLenum, GLint>> m_uniform_map;
	std::unordered_map<std::string, std::tuple<GLuint, GLint>> m_uniform_block_map;

	GLint m_max_uniform_buffer_bindings = 0;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::program
/// \ingroup objects
///
/// A gls::program is an object that encapsulates an OpenGL program object. Like
/// all objects in GLS, the underlying name is generated at object construction
/// and deleted at destruction. This can be retrieved with name().
///
/// In order to use a gls::program, you need to link successfully compiled
/// gls::shader objects to it. If linking fails, you can check the information
/// log for the cause. Upon successful linking, all attribute, uniform and
/// uniform block information is queried and cached within the gls::program.
///
/// To retrieve attribute and uniform locations, use the
/// get_attribute_location() and get_uniform_location() methods. In addition to
/// that, you can query the size and data type of each attribute and uniform.
///
/// When successfully linked, the gls::program also automatically maps each
/// active uniform block to its own binding point. If there are more active
/// uniform blocks than there are bindings supported on the hardware, only
/// the first blocks that are reported by the GL which can fit are bound. When
/// running in a debug configuration, a warning should also be output to the
/// standard error stream.
///
/// To set uniform values, a thin convenience wrapper is provided on top of the
/// OpenGL interface. Where possible, overloading is used instead of explicitly
/// naming the parameter types as part of the method name.
///
/// To use a gls::program, simply call use(). It will set the gls::program as
/// the active one and replace any previously active program. To clear the
/// active program, call unuse().
///
/// Example usage:
/// \code
/// ... compile your shaders ...
///
/// auto my_program = gls::program();
/// my_program.link( my_vertex_shader, my_fragment_shader );
///
/// ... set up attributes ...
///
/// my_program.uniform( "my_uniform", 1.f, 2.f, 3.f );
/// my_program.use();
///
/// ... draw stuff ...
///
/// my_program.unuse();
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
