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
	void gen_renderbuffers( GLsizei size, GLuint* name ) { glGenRenderbuffers( size, name ); }
	void delete_renderbuffers( GLsizei size, const GLuint* name ) { glDeleteRenderbuffers( size, name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL renderbuffer object
///
////////////////////////////////////////////////////////////////////////////////
class renderbuffer {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Construct a gls::renderbuffer without multi-sampling
	///
	/// Renderbuffer storage is automatically allocated.
	///
	/// \param internal_format Internal format of the renderbuffer storage
	/// \param the_width Width of the renderbuffer storage
	/// \param the_height Height of the renderbuffer storage
	///
	//////////////////////////////////////////////////////////////////////////////
	renderbuffer( GLenum internal_format, GLsizei the_width, GLsizei the_height ) :
		renderbuffer( 0, internal_format, the_width, the_height )
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Construct a gls::renderbuffer with multi-sampling
	///
	/// Renderbuffer storage is automatically allocated.
	///
	/// \param samples Number of samples to use for the renderbuffer storage
	/// \param internal_format Internal format of the renderbuffer storage
	/// \param the_width Width of the renderbuffer storage
	/// \param the_height Height of the renderbuffer storage
	///
	//////////////////////////////////////////////////////////////////////////////
	renderbuffer( GLsizei samples, GLenum internal_format, GLsizei the_width, GLsizei the_height ) :
		m_width( the_width ),
		m_height( the_height )
	{
		auto max_samples = GLint();
		auto max_renderbuffer_size = GLint();

		check_gl_error( glGetIntegerv( GL_MAX_SAMPLES, &max_samples ) );
		check_gl_error( glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size ) );

		assert( samples <= static_cast<GLsizei>( max_samples ) );
		assert( the_width <= static_cast<GLsizei>( max_renderbuffer_size ) );
		assert( the_height <= static_cast<GLsizei>( max_renderbuffer_size ) );

		bind();
		check_gl_error( glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, internal_format, the_width, the_height ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this renderbuffer
	///
	/// \return OpenGL name of this renderbuffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the width of the allocated renderbuffer storage
	///
	/// \return Width of the allocated renderbuffer storage
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsizei width() const {
		return m_width;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the height of the allocated renderbuffer storage
	///
	/// \return Height of the allocated renderbuffer storage
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsizei height() const {
		return m_width;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind this renderbuffer
	///
	//////////////////////////////////////////////////////////////////////////////
	void bind() {
		check_gl_error( glBindRenderbuffer( GL_RENDERBUFFER, m_object.name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the currently bound renderbuffer
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unbind() {
		check_gl_error( glBindRenderbuffer( GL_RENDERBUFFER, 0 ) );
	}

private:
	object<priv::gen_renderbuffers, priv::delete_renderbuffers> m_object;
	GLsizei m_width;
	GLsizei m_height;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::renderbuffer
/// \ingroup objects
///
/// A gls::renderbuffer is an object that encapsulates an OpenGL renderbuffer
/// object. Like all objects in GLS, the underlying name is generated at object
/// construction and deleted at destruction. This can be retrieved with name().
/// Additionally, the storage for the renderbuffer is automatically allocated at
/// construction as well.
///
/// gls::renderbuffer objects are only used as part of gls::framebuffer objects.
/// If you want, you can also manually attach the gls::renderbuffer object to
/// your own framebuffer object using name().
///
/// To bind a gls::renderbuffer, simply call bind(). It will bind the
/// gls::renderbuffer to the GL_RENDERBUFFER binding point and replace any
/// previously bound to that target. To clear the binding to that target, call
/// unbind().
///
/// Example usage:
/// \code
/// auto framebuffer = gls::framebuffer();
/// framebuffer.add_renderbuffer( GL_DEPTH_ATTACHMENT, gls::renderbuffer( GL_DEPTH_COMPONENT24, 100, 100 ) );
/// framebuffer.bind();
///
/// ... draw stuff to the depth renderbuffer ...
///
/// framebuffer.unbind();
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
