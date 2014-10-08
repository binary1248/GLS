/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/object.hpp>
#include <gls/objects/renderbuffer.hpp>
#include <type_traits>
#include <unordered_map>
#include <cassert>

namespace gls {

namespace priv {
	void gen_framebuffers( GLsizei size, GLuint* name ) { glGenFramebuffers( size, name ); }
	void delete_framebuffers( GLsizei size, const GLuint* name ) { glDeleteFramebuffers( size, name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL framebuffer object
///
////////////////////////////////////////////////////////////////////////////////
class framebuffer {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this framebuffer
	///
	/// \return OpenGL name of this framebuffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind this framebuffer to the GL_FRAMEBUFFER target
	///
	/// If you want to bind this framebuffer to a target other than
	/// GL_FRAMEBUFFER, you can always do this manually with:
	/// \code
	/// glBindFramebuffer( <your target>, framebuffer.name() );
	/// \endcode
	///
	//////////////////////////////////////////////////////////////////////////////
	void bind() {
		check_gl_error( glBindFramebuffer( GL_FRAMEBUFFER, m_object.name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the framebuffer currently bound to the GL_FRAMEBUFFER target
	///
	/// If you want to unbind a framebuffer that is bound to a target other than
	/// GL_FRAMEBUFFER, you can always do this manually with:
	/// \code
	/// glBindFramebuffer( <your target>, 0 );
	/// \endcode
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unbind() {
		check_gl_error( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Attach a non-array cubemap to the given attachment
	///
	/// \param attachment Attachment to attach to
	/// \param textarget Cubemap texture target (GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, etc.)
	/// \param the_texture The cubemap texture to attach
	/// \param level Level of the cubemap texture to attach
	///
	//////////////////////////////////////////////////////////////////////////////
	void attach_texture( GLenum attachment, GLenum textarget, const texture<GL_TEXTURE_CUBE_MAP>& the_texture, GLint level ) {
		bind();
		check_gl_error( glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, textarget, the_texture.name(), level ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Attach a texture to the given attachment
	///
	/// \param attachment Attachment to attach to
	/// \param the_texture The texture to attach
	/// \param level Level of the texture to attach
	///
	//////////////////////////////////////////////////////////////////////////////
	template<GLenum Target>
	void attach_texture( GLenum attachment, const texture<Target>& the_texture, GLint level ) {
		static_assert( Target != GL_TEXTURE_CUBE_MAP, "glFramebufferTexture cannot take non-array cubemaps" );

		bind();
		check_gl_error( glFramebufferTexture( GL_FRAMEBUFFER, attachment, the_texture.name(), level ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Attach a texture layer to the given attachment
	///
	/// \param attachment Attachment to attach to
	/// \param the_texture The texture to attach
	/// \param level Level of the texture to attach
	/// \param layer Layer of the texture to attach
	///
	//////////////////////////////////////////////////////////////////////////////
	template<GLenum Target>
	void attach_texture_layer( GLenum attachment, const texture<Target>& the_texture, GLint level, GLint layer ) {
		static_assert( Target != GL_TEXTURE_CUBE_MAP, "glFramebufferTextureLayer cannot take non-array cubemaps" );

		bind();
		check_gl_error( glFramebufferTextureLayer( GL_FRAMEBUFFER, attachment, the_texture.name(), level, layer ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Clear a texture attachment
	///
	/// \param attachment Attachment to detach a texture from
	///
	//////////////////////////////////////////////////////////////////////////////
	void detach_texture( GLenum attachment ) {
		bind();
		check_gl_error( glFramebufferTexture( GL_FRAMEBUFFER, attachment, 0, 0 ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Attach a renderbuffer to the given attachment
	///
	/// Note: This method requires that you transfer ownership (i.e. std::move or
	/// equivalent) of the renderbuffer to the framebuffer. It will be destroyed
	/// either when the attachment is cleared or the gls::framebuffer is
	/// destroyed.
	///
	/// \param attachment Attachment to attach to
	/// \param the_renderbuffer The renderbuffer to attach
	///
	//////////////////////////////////////////////////////////////////////////////
	void add_renderbuffer( GLenum attachment, renderbuffer&& the_renderbuffer ) {
		bind();
		check_gl_error( glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, the_renderbuffer.name() ) );
		m_renderbuffers.emplace( attachment, std::forward<renderbuffer>( the_renderbuffer ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Clear a renderbuffer attachment
	///
	/// \param attachment Attachment to detach a renderbuffer from
	///
	//////////////////////////////////////////////////////////////////////////////
	void remove_renderbuffer( GLenum attachment ) {
		bind();
		check_gl_error( glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0 ) );
		m_renderbuffers.erase( attachment );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Check the status of the framebuffer
	///
	/// \return Status of the framebuffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLenum status() {
		bind();
		auto status_value = check_gl_error( glCheckFramebufferStatus( GL_FRAMEBUFFER ) );
		unbind();

		return status_value;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Check if the framebuffer is complete
	///
	/// \return true if the framebuffer is complete, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	bool complete() {
		return status() == GL_FRAMEBUFFER_COMPLETE;
	}

private:
	std::unordered_map<GLenum, renderbuffer> m_renderbuffers;
	object<priv::gen_framebuffers, priv::delete_framebuffers> m_object;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::framebuffer
/// \ingroup objects
///
/// A gls::framebuffer is an object that encapsulates an OpenGL framebuffer
/// object. Like all objects in GLS, the underlying name is generated at object
/// construction and deleted at destruction. This can be retrieved with name().
///
/// \b WARNING: Unlike other OpenGL objects, framebuffer objects are \a not
/// shareable between contexts. As such, a gls::framebuffer object is only
/// valid for use within the context where it was constructed. No checks are
/// made to ensure it is used properly. If strange behaviour occurs, be sure to
/// check the reported OpenGL errors in a debug configuration and assure you are
/// not transferring the gls::framebuffer between contexts.
///
/// Framebuffer objects are the primary way of rendering to an offscreen surface
/// on hardware that supports it. By default, i.e. when 0 is bound to
/// GL_FRAMEBUFFER all draw commands affect the default framebuffer, typically
/// the back buffer in double buffered applications. To draw to a framebuffer
/// object instead, it has to be bound. All draw commands that are issued while
/// it is bound will affect the framebuffer and its attachments. Once you want
/// to draw to the back buffer once again, simply unbind the framebuffer.
///
/// The framebuffer attachments represent surfaces on which draw commands will
/// have an affect. The depth attachment will have depth values written to it
/// while the colour attachments will have colour values written to it and so
/// on. The storage for these attachments comes either from standard texture
/// objects or, since texture objects cannot store non-colour values,
/// renderbuffer objects. The gls::framebuffer object will take ownership of any
/// gls::renderbuffer objects that are attached to it. After all draw commands
/// affecting the framebuffer have completed, one can use the attached texture
/// objects much like one would use any other texture object. Just beware of
/// <a href="https://www.opengl.org/wiki/Memory_Model#Framebuffer_objects">
/// feedback loops</a>.
///
/// After assembling a framebuffer from its attachments and before rendering to
/// it, it is always a good idea to check whether it is complete. This can be
/// done simply by calling complete(). If it is incomplete, calling status()
/// will return the reason for its incompleteness.
///
/// To render to a gls::framebuffer, simply call bind(). It will bind the
/// gls::framebuffer to the GL_FRAMEBUFFER binding and replace any previously
/// bound to that target. To clear the GL_FRAMEBUFFER binding and render to the
/// back buffer again, call unbind().
///
/// Example usage:
/// \code
/// auto texture = gls::texture<GL_TEXTURE_2D>();
/// texture.image_2d( 0, GL_RGBA, 100, 100, GL_RGBA, GL_FLOAT, nullptr );
/// auto framebuffer = gls::framebuffer();
/// framebuffer.attach_texture( GL_COLOR_ATTACHMENT0, texture, 0 );
/// framebuffer.add_renderbuffer( GL_DEPTH_ATTACHMENT, gls::renderbuffer( GL_DEPTH_COMPONENT24, 100, 100 ) );
/// if( !framebuffer.complete() ) {
/// 	... framebuffer is not complete ...
/// }
///
/// framebuffer.bind();
///
/// ... draw stuff to the framebuffer ...
///
/// framebuffer.unbind();
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
