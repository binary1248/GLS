/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/object.hpp>
#include <gls/objects/buffer.hpp>
#include <cassert>

namespace gls {

namespace priv {
	void gen_vertex_arrays( GLsizei size, GLuint* name ) { glGenVertexArrays( size, name ); }
	void delete_vertex_arrays( GLsizei size, const GLuint* name ) { glDeleteVertexArrays( size, name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL vertex array object
///
////////////////////////////////////////////////////////////////////////////////
class vertexarray {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this vertex array
	///
	/// \return OpenGL name of this vertex array
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind this vertex array
	///
	//////////////////////////////////////////////////////////////////////////////
	void bind() {
		check_gl_error( glBindVertexArray( m_object.name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the currently bound vertex array
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unbind() {
		check_gl_error( glBindVertexArray( 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind a shader attribute to a buffer source
	///
	/// Shader attributes need to have a data source to provide the required
	/// attribute data to a vertex shader. This method binds an attribute with the
	/// given name in the given program to the given buffer source. Since
	/// attributes may be interleaved within the same buffer, providing a non-zero
	/// stride and offset might be necessary as well.
	///
	/// \param the_program The program in which the attribute resides
	/// \param attribute_name The name of the attribute to bind
	/// \param the_buffer The buffer to source data from
	/// \param size The size (number of components) of an attribute
	/// \param type The data type of an attribute component
	/// \param normalized Whether to normalize integer data to the floating point range [0.f, 1.f] for unsigned or [-1.f, 1.f] for signed in the shader
	/// \param stride Number of bytes between successive attribute elements in the buffer
	/// \param offset Byte offset into the buffer where the first element is located
	///
	//////////////////////////////////////////////////////////////////////////////
	template<GLenum BufferTarget, GLenum BufferUsage, typename T, typename U>
	void bind_attribute( const program& the_program, const std::string& attribute_name, const buffer<BufferTarget, BufferUsage>& the_buffer, GLint size, GLenum type, GLboolean normalized, T stride, U offset ) {
		auto attribute_location = the_program.get_attribute_location( attribute_name );

		if( attribute_location < 0 ) {
			return;
		}

		bind_attribute( static_cast<GLuint>( attribute_location ), the_buffer, size, type, normalized, stride, offset );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind a shader attribute to a buffer source
	///
	/// Shader attributes need to have a data source to provide the required
	/// attribute data to a vertex shader. This method binds an attribute with the
	/// given location to the given buffer source. Since attributes may be
	/// interleaved within the same buffer, providing a non-zero stride and offset
	/// might be necessary as well.
	///
	/// \param attribute_location Location of the attribute
	/// \param the_buffer The buffer to source data from
	/// \param size The size (number of components) of an attribute
	/// \param type The data type of an attribute component
	/// \param normalized Whether to normalize integer data to the floating point range [0.f, 1.f] for unsigned or [-1.f, 1.f] for signed in the shader
	/// \param stride Number of bytes between successive attribute elements in the buffer
	/// \param offset Byte offset into the buffer where the first element is located
	///
	//////////////////////////////////////////////////////////////////////////////
	template<GLenum BufferTarget, GLenum BufferUsage, typename T, typename U>
	void bind_attribute( GLuint attribute_location, const buffer<BufferTarget, BufferUsage>& the_buffer, GLint size, GLenum type, GLboolean normalized, T stride, U offset ) {
		bind();
		check_gl_error( glBindBuffer( GL_ARRAY_BUFFER, the_buffer.name() ) );
		check_gl_error( glEnableVertexAttribArray( attribute_location ) );
		check_gl_error( glVertexAttribPointer( attribute_location, size, type, normalized, static_cast<GLsizei>( stride ), reinterpret_cast<const GLvoid*>( offset ) ) );
		check_gl_error( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
		unbind();
		check_gl_error( glDisableVertexAttribArray( attribute_location ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind a shader attribute
	///
	/// This method disables the vertex attribute with the given name in the given
	/// program from sourcing its data from a buffer. If the attribute is present
	/// in the shader, it will be set to a constant value for every invocation.
	///
	/// \param the_program The program in which the attribute resides
	/// \param attribute_name The name of the attribute to unbind
	///
	//////////////////////////////////////////////////////////////////////////////
	void unbind_attribute( const program& the_program, const std::string& attribute_name ) {
		auto attribute_location = the_program.get_attribute_location( attribute_name );

		if( attribute_location < 0 ) {
			return;
		}

		unbind_attribute( static_cast<GLuint>( attribute_location ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind a shader attribute
	///
	/// This method disables the vertex attribute with the given location from
	/// sourcing its data from a buffer. If the attribute is present in the
	/// shader, it will be set to a constant value for every invocation.
	///
	/// \param attribute_location The location of the attribute to unbind
	///
	//////////////////////////////////////////////////////////////////////////////
	void unbind_attribute( GLuint attribute_location ) {
		bind();
		check_gl_error( glDisableVertexAttribArray( static_cast<GLuint>( attribute_location ) ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Clear all attribute bindings
	///
	/// Calling this method is equivalent to this code:
	/// \code
	/// auto max_vertex_attributes = GLint();
	/// check_gl_error( glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &max_vertex_attributes ) );
	/// vertexarray.bind();
	/// for( GLuint index = 0; index < max_vertex_attributes; ++index ) {
	/// 	glDisableVertexAttribArray( index );
	/// }
	/// vertexarray.unbind();
	/// \endcode
	///
	//////////////////////////////////////////////////////////////////////////////
	void clear_attribute_bindings() {
		auto max_vertex_attributes = GLint();

		check_gl_error( glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &max_vertex_attributes ) );

		bind();

		for( GLuint index = 0; index < static_cast<GLuint>( max_vertex_attributes ); ++index ) {
			check_gl_error( glDisableVertexAttribArray( index ) );
		}

		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind an index buffer to this vertex array
	///
	/// When doing indexed rendering, it is required that you bind a buffer
	/// containing the indices of the vertices to be rendered to the element
	/// array buffer binding within this vertex array. That is what this method
	/// does.
	///
	/// \param the_buffer The index buffer to bind to this vertex array
	///
	//////////////////////////////////////////////////////////////////////////////
	template<GLenum BufferTarget, GLenum BufferUsage>
	void bind_index_buffer( const buffer<BufferTarget, BufferUsage>& the_buffer ) {
		bind();
		check_gl_error( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, the_buffer.name() ) );
		unbind();
		check_gl_error( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the index buffer from this vertex array
	///
	/// This will clear the element array buffer binding within this vertex array.
	///
	//////////////////////////////////////////////////////////////////////////////
	void unbind_index_buffer() {
		bind();
		check_gl_error( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
		unbind();
	}

private:
	object<priv::gen_vertex_arrays, priv::delete_vertex_arrays> m_object;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::vertexarray
/// \ingroup objects
///
/// A gls::vertexarray is an object that encapsulates an OpenGL vertex array
/// object. Like all objects in GLS, the underlying name is generated at object
/// construction and deleted at destruction. This can be retrieved with name().
///
/// \b WARNING: Unlike other OpenGL objects, vertex array objects are \a not
/// shareable between contexts. As such, a gls::vertexarray object is only
/// valid for use within the context where it was constructed. No checks are
/// made to ensure it is used properly. If strange behaviour occurs, be sure to
/// check the reported OpenGL errors in a debug configuration and assure you are
/// not transferring the gls::vertexarray between contexts.
///
/// Vertex array objects are basically a way to store the layout and sources of
/// attribute data. While multiple programs can use the same vertex array, it
/// makes less sense to use different vertex arrays to reference the same data.
/// Vertex arrays are meant to be set up once and reused many times. They only
/// save the \a source of vertex attributes and not the data themselves. This
/// means that it is not necessary to update a vertex array if the data within a
/// buffer object is changed. It \a does mean however, that if a buffer object
/// is recreated, deleted, or has its name changed, you will need to
/// rebind/unbind the attribute(s) sourcing from that buffer object.
///
/// In addition to storing the layout and sources of attribute data, vertex
/// arrays also store the element array binding. This means that you can store a
/// reference to an index buffer within the vertex array as well. The same
/// applies here as well however: If the index buffer object is recreated,
/// deleted, or has its name changed, you will need to rebind/unbind it to/from
/// the vertex array.
///
/// To use a gls::vertexarray, simply call bind(). It will bind the
/// gls::vertexarray as the current one used for rendering and replace any that
/// was previously bound. To clear the binding, call unbind(). It is an error
/// in newer versions of OpenGL to render without a bound vertex array.
///
/// Example usage:
/// \code
/// // Create and link the program from our compiled shaders
/// auto program = gls::program();
/// program.link( vertex_shader, fragment_shader );
///
/// auto position_buffer = gls::buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>();
/// auto position_data = std::vector<float>();
/// position_data = { -.5f, -.5f, -1.f, .5f, -.5f, -1.f, 0.f, .5f, -1.f };
/// position_buffer.data( position_data.size() * sizeof( float ), position_data.data() );
///
/// auto vertexarray = gls::vertexarray();
///
/// // Bind a "position" attribute in the vertex shader to our position buffer
/// vertexarray.bind_attribute( program, "position", position_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0 );
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
