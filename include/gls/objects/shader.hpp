/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/object.hpp>

#if !defined( NDEBUG )
	#if !defined( GLS_ERROR_STREAM )
		#include <iostream>
		#define GLS_ERROR_STREAM std::cerr
	#endif
#endif

namespace gls {

namespace priv {
	template<GLenum Type>
	void create_shader( GLsizei, GLuint* name ) { *name = glCreateShader( Type ); }
	void delete_shader( GLsizei, const GLuint* name ) { glDeleteShader( *name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL shader object
///
/// \tparam Type The type of this shader (e.g. GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
///
////////////////////////////////////////////////////////////////////////////////
template<GLenum Type>
class shader {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this shader
	///
	/// \return OpenGL name of this shader
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Compile this shader from the given source
	///
	/// This will attempt to compile the shader using the given source. Checking
	/// the produced information log with get_info_log() can help to solve errors
	/// that occur or warnings the driver might have produced.
	///
	/// After successfully compiling a set of shaders, link them into a
	/// gls::program object in order to use them as part of the program.
	///
	/// \param source String containing the source to compile
	/// \return true if compiling was successful, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	bool compile( const std::string& source ) {
		auto c_str = source.c_str();

		check_gl_error( glShaderSource( name(), 1, reinterpret_cast<const GLchar**>( &c_str ), nullptr ) );
		check_gl_error( glCompileShader( name() ) );

		auto compile_status = GL_FALSE;
		check_gl_error( glGetShaderiv( name(), GL_COMPILE_STATUS, &compile_status ) );

#if !defined( NDEBUG )
		auto info_log = get_info_log();

		if( !info_log.empty() ) {
			GLS_ERROR_STREAM << std::move( info_log );
		}
#endif

		return compile_status == GL_TRUE;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the link information log
	///
	/// After compiling, whether successful or not, an information log might be
	/// available. Check this information log for warnings after successful
	/// compiling, or errors after failed compiling. The information log may be
	/// empty if compiling was successful.
	///
	/// \return String containing the information log
	///
	//////////////////////////////////////////////////////////////////////////////
	std::string get_info_log() const {
		auto info_log_length = GLint();
		check_gl_error( glGetShaderiv( name(), GL_INFO_LOG_LENGTH, &info_log_length ) );

		if( !info_log_length ) {
			return std::string();
		}

		auto info_log = std::vector<GLchar>( static_cast<std::size_t>( info_log_length ), 0 );
		check_gl_error( glGetShaderInfoLog( name(), info_log_length, nullptr, info_log.data() ) );

		return std::string( static_cast<const char*>( info_log.data() ) );
	}

private:
	object<priv::create_shader<Type>, priv::delete_shader> m_object;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::shader
/// \ingroup objects
///
/// A gls::shader is an object that encapsulates an OpenGL shader object. It is
/// used along with other gls::shader objects as part of a gls::program object.
/// Like all objects in GLS, the underlying name is generated at object
/// construction and deleted at destruction. This can be retrieved with name().
///
/// In order to link a gls::shader object with other gls::shader objects as part
/// of a gls::program, you need to have successfully compiled them all. If
/// compiling fails, you can check the information log for the cause.
///
/// Example usage:
/// \code
/// auto vertex_shader = gls::shader<GL_VERTEX_SHADER>();
/// if( !vertex_shader.compile( R"(
/// 	#version 150 core
///
/// 	in vec3 position;
///
/// 	void main() {
/// 		gl_Position = vec4( position, 1.0 );
/// 	}
/// )" ) ) {
/// 	... vertex shader failed to compile ...
/// }
///
/// auto fragment_shader = gls::shader<GL_FRAGMENT_SHADER>();
/// if( !fragment_shader.compile( R"(
/// 	#version 150 core
///
/// 	out vec4 frag_color;
///
/// 	void main() {
/// 		frag_color = vec4( 1.0, 1.0, 1.0, 1.0 );
/// 	}
/// )" ) ) {
/// 	... fragment shader failed to compile ...
/// }
///
/// auto program = gls::program();
/// if( !program.link( vertex_shader, fragment_shader ) ) {
/// 	... program failed to link ...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
