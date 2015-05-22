/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/object.hpp>
#include <cassert>

namespace gls {

namespace priv {
	void gen_buffers( GLsizei size, GLuint* name ) { glGenBuffers( size, name ); }
	void delete_buffers( GLsizei size, const GLuint* name ) { glDeleteBuffers( size, name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL buffer object
///
/// \tparam Target The target this buffer object will be bound to when calling bind()
/// \tparam Usage The usage hint passed to OpenGL when the buffer storage is created
///
////////////////////////////////////////////////////////////////////////////////
template<GLenum Target = GL_ARRAY_BUFFER, GLenum Usage = GL_DYNAMIC_DRAW>
class buffer {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this buffer
	///
	/// \return OpenGL name of this buffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the size of this buffer
	///
	/// After calling data(), storage is allocated for the buffer by the GL. This
	/// method returns how much storage was requested during the last allocation
	/// that took place.
	///
	/// \return Size of this buffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsizeiptr size() const {
		return m_size;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind this buffer to its target
	///
	/// Bind this buffer to the target specified by the Target template parameter.
	/// This replaces any previous binding on that target.
	///
	//////////////////////////////////////////////////////////////////////////////
	void bind() {
		check_gl_error( glBindBuffer( Target, m_object.name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the current buffer from the target
	///
	/// Unbind the buffer currently bound to the target specified by the Target
	/// template parameter.
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unbind() {
		check_gl_error( glBindBuffer( Target, 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind a range to an indexed target
	///
	/// Bind a range given by an offset into this buffer along with the size of
	/// the range in bytes to an indexed buffer target. The target is specified by
	/// the Target template parameter.
	///
	/// \param index Index of the binding point within the target
	/// \param offset Byte offset into the buffer where the range starts
	/// \param range_size Size of the range in bytes
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void bind_range( GLuint index, GLintptr offset, T range_size ) {
		check_gl_error( glBindBufferRange( Target, index, m_object.name(), offset, static_cast<GLsizeiptr>( range_size ) ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Allocate storage and upload data
	///
	/// Request the GL to allocate storage for this buffer specifying usage
	/// according to the Usage template parameter and optionally upload data to
	/// it. If no data should currently be uploaded, pass a nullptr as the second
	/// argument.
	///
	/// Even if the new size is equal to the previously allocated size, this
	/// method will orphan the buffer. If this is not wanted, use sub_data()
	/// instead.
	///
	/// If you only want to partially fill the buffer with data, pass nullptr as
	/// the second argument and call sub_data() after this.
	///
	/// \param data_size Size in bytes of the storage to be allocated
	/// \param data_ptr Pointer to a data source to upload from, or nullptr if no upload should take place
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void data( T data_size, const GLvoid* data_ptr ) {
		assert( data_size >= 0 );
		m_size = static_cast<GLsizeiptr>( data_size );

		bind();
		check_gl_error( glBufferData( Target, m_size, data_ptr, Usage ) );

		if( data_ptr ) {
			sub_data( 0, m_size, data_ptr );
		}

		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Upload a data range
	///
	/// Upload a range of data to the buffer at the given byte offset. If the
	/// current buffer is not large enough to hold all the data, a new buffer will
	/// be created and will replace the current one. Be aware that if this
	/// reallocation takes place, the name of the underlying buffer object will be
	/// changed, and updating all references to this buffer may be necessary.
	///
	/// \param offset Byte offset into the buffer where the data should be uploaded
	/// \param data_size Size of the range of data to be uploaded in bytes
	/// \param data_ptr Pointer to a data source to upload from
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void sub_data( GLintptr offset, T data_size, const GLvoid* data_ptr ) {
		if( m_size < offset + static_cast<GLsizeiptr>( data_size ) ) {
			buffer new_buffer;

			new_buffer.data( offset + static_cast<GLsizeiptr>( data_size ), nullptr );
			new_buffer.copy_sub_data( *this, 0, 0, m_size );

			std::swap( *this, new_buffer );
		}

		bind();
		check_gl_error( glBufferSubData( Target, offset, static_cast<GLsizeiptr>( data_size ), data_ptr ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Copy a data range from another buffer into this one
	///
	/// This copies a range of data of the specified size in bytes from the read
	/// offset in bytes from the source buffer to this buffer at the write offset
	/// in bytes. If the current buffer is not large enough to hold all the data,
	/// a new buffer will be created and will replace the current one. Be aware
	/// that if this reallocation takes place, the name of the underlying buffer
	/// object will be changed, and updating all references to this buffer may be
	/// necessary.
	///
	/// Because this data transfer takes place on the server (GPU), this avoids
	/// any expensive read backs that would be incurred if done manually using a
	/// combination of the other methods.
	///
	/// \param source Source buffer to copy data from
	/// \param readoffset Offset in bytes of the start of the range to read from the source buffer
	/// \param writeoffset Offset in bytes to start writing the range to in this buffer
	/// \param data_size Size in bytes of the range to copy
	///
	//////////////////////////////////////////////////////////////////////////////
	template<GLenum SourceTarget, GLenum SourceUsage, typename V>
	void copy_sub_data( const buffer<SourceTarget, SourceUsage>& source, GLintptr readoffset, GLintptr writeoffset, V data_size ) {
		assert( data_size > 0 );
		assert( source.size() >= readoffset + static_cast<GLsizeiptr>( data_size ) );

		if( m_size < writeoffset + static_cast<GLsizeiptr>( data_size ) ) {
			buffer new_buffer;

			new_buffer.data( writeoffset + static_cast<GLsizeiptr>( data_size ), nullptr );

			if( m_size ) {
				new_buffer.copy_sub_data( *this, 0, 0, m_size );
			}

			std::swap( *this, new_buffer );
		}

		check_gl_error( glBindBuffer( GL_COPY_READ_BUFFER, source.name() ) );
		check_gl_error( glBindBuffer( GL_COPY_WRITE_BUFFER, name() ) );

		check_gl_error( glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, readoffset, writeoffset, static_cast<GLsizeiptr>( data_size ) ) );

		check_gl_error( glBindBuffer( GL_COPY_READ_BUFFER, 0 ) );
		check_gl_error( glBindBuffer( GL_COPY_WRITE_BUFFER, 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Read back data from the buffer
	///
	/// This reads a range of data from the buffer back into client memory.
	///
	/// \param offset Offset in bytes into the buffer to read the range from
	/// \param data_size Size in bytes of the range
	/// \param data_ptr Pointer to client memory that is large enough to hold the data
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void get_sub_data( GLintptr offset, T data_size, GLvoid* data_ptr ) {
		assert( m_size >= offset + static_cast<GLsizeiptr>( data_size ) );

		bind();
		check_gl_error( glGetBufferSubData( Target, offset, static_cast<GLsizeiptr>( data_size ), data_ptr ) );
		unbind();
	}

private:
	object<priv::gen_buffers, priv::delete_buffers> m_object;
	GLsizeiptr m_size = 0;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::buffer
/// \ingroup objects
///
/// A gls::buffer is an object that encapsulates an OpenGL buffer object. Like
/// all objects in GLS, the underlying name is generated at object construction
/// and deleted at destruction. This can be retrieved with name().
///
/// Buffer objects store arbitrary data within server (GPU) memory. This buffer
/// can then be used to e.g. provide attribute data to shaders or act as the
/// data storage for uniform blocks.
///
/// All the provided methods offer different ways of manipulating the buffer's
/// data content. One must be aware of the performance implications some
/// transfers might have. As a general rule of thumb, try to avoid unnecessary
/// transfers between client and server, and re-use data within the buffers if
/// possible.
///
/// To use a gls::buffer, simply call bind(). It will bind the gls::buffer to
/// its target binding and replace any previously bound to that target. To
/// clear the binding to that target, call unbind().
///
/// Example usage:
/// \code
/// auto buffer1 = gls::buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>();
///
/// auto write_data = std::vector<float>();
/// write_data = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f };
/// buffer1.data( write_data.size() * sizeof( float ), write_data.data() );
///
/// auto buffer2 = gls::buffer<GL_UNIFORM_BUFFER, GL_STREAM_DRAW>();
/// buffer2.copy_sub_data( buffer1, 0, 0, buffer1.size() );
///
/// write_data = { 0.f, 0.f, 0.f };
/// buffer2.sub_data( 3 * sizeof( float ), write_data.size() * sizeof( float ), write_data.data() );
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
