/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/buffer.hpp>
#include <gls/objects/texture.hpp>

namespace gls {

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL buffer object
///
/// \tparam InternalFormat The internal format used by this buffer texture
///
////////////////////////////////////////////////////////////////////////////////
template<GLenum InternalFormat>
class buffertexture : public buffer<GL_TEXTURE_BUFFER, GL_STREAM_DRAW>, public texture<GL_TEXTURE_BUFFER> {
public:
	buffertexture() :
		texture<GL_TEXTURE_BUFFER>( 0 )
	{
		data( 0, nullptr );
		texture::bind();
		check_gl_error( glTexBuffer( GL_TEXTURE_BUFFER, InternalFormat, buffer::name() ) );
		texture::unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of the buffer
	///
	/// \return OpenGL name of the buffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint buffer_name() const {
		return buffer::name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of the texture
	///
	/// \return OpenGL name of the texture
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint texture_name() const {
		return texture::name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind this buffer texture
	///
	/// This replaces any buffer texture previously bound to the GL_TEXTURE_BUFFER
	/// binding point.
	///
	//////////////////////////////////////////////////////////////////////////////
	void bind() {
		texture::bind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the current buffer texture
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unbind() {
		texture::unbind();
	}
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::buffertexture
/// \ingroup objects
///
/// A gls::buffertexture is an object that encapsulates an OpenGL buffer texture
/// object. Unlike other objects in GLS, this simply acts as an interface to a
/// texture object and a buffer object that is used as its storage. As such, any
/// gls::buffertexture object can be used like a normal gls::buffer object
/// created with GL_TEXTURE_BUFFER and GL_STREAM_DRAW template parameters and a
/// normal gls::texture object created with the GL_TEXTURE_BUFFER target.
///
/// The names of the underlying objects can be retrieved with buffer_name() and
/// texture_name().
///
/// Buffer textures provide a way for samplers to source their data directly
/// from a buffer instead of conventional texture storage. This typically allows
/// for much more data to be available to a shader than if one were to rely
/// solely on uniform storage, whose capacity is an order of magnitude smaller
/// than buffer capacity. However, this also means that the shader will have to
/// fetch more data using a possibly slower path than if it were to use uniforms
/// instead.
///
/// Example usage:
/// \code
/// auto buffertexture = gls::buffertexture<GL_R32F>();
/// auto write_data = std::vector<float>();
/// write_data = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f };
/// buffertexture.data( write_data.size() * sizeof( float ), write_data.data() );
///
/// buffertexture.bind();
///
/// ... do stuff that makes use of samplers that sample from the buffer texture ...
///
/// buffertexture.unbind();
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
