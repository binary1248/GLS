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
	void gen_textures( GLsizei size, GLuint* name ) { glGenTextures( size, name ); }
	void delete_textures( GLsizei size, const GLuint* name ) { glDeleteTextures( size, name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL texture object
///
/// \tparam Target The target this buffer object will be bound to when calling bind()
///
////////////////////////////////////////////////////////////////////////////////
template<GLenum Target = GL_TEXTURE_2D>
class texture {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///
	/// The constructor, after a name has been generated for the object, sets the
	/// GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER texture parameters to
	/// GL_LINEAR.
	///
	//////////////////////////////////////////////////////////////////////////////
	texture() {
		parameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		parameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this texture
	///
	/// \return OpenGL name of this texture
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the width of the level 0 image of the allocated texture storage
	///
	/// \return Width of the level 0 image of the allocated texture storage
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsizei width() const {
		return m_width;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the height of the level 0 image of the allocated texture storage
	///
	/// \return Height of the level 0 image of the allocated texture storage
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsizei height() const {
		return m_height;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the depth of the level 0 image of the allocated texture storage
	///
	/// \return Depth of the level 0 image of the allocated texture storage
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsizei depth() const {
		return m_depth;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Bind this texture to its target
	///
	/// Bind this texture to the target specified by the Target template
	/// parameter. This replaces any previous binding on that target.
	///
	//////////////////////////////////////////////////////////////////////////////
	void bind() {
		check_gl_error( glBindTexture( Target, m_object.name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Unbind the current texture from the target
	///
	/// Unbind the texture currently bound to the target specified by the Target
	/// template parameter.
	///
	//////////////////////////////////////////////////////////////////////////////
	static void unbind() {
		check_gl_error( glBindTexture( Target, 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a texture parameter
	///
	/// Set the given parameter to the given value
	///
	/// \param pname Name of the parameter to set
	/// \param param Value to set the parameter to
	///
	//////////////////////////////////////////////////////////////////////////////
	void parameter( GLenum pname, GLfloat param ) {
		bind();
		check_gl_error( glTexParameterf( Target, pname, param ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a texture parameter
	///
	/// Set the given parameter to the given value
	///
	/// \param pname Name of the parameter to set
	/// \param param Value to set the parameter to
	///
	//////////////////////////////////////////////////////////////////////////////
	void parameter( GLenum pname, GLint param ) {
		bind();
		check_gl_error( glTexParameteri( Target, pname, param ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a texture parameter
	///
	/// Set the given parameter to the given value
	///
	/// \param pname Name of the parameter to set
	/// \param param Pointer to the value to set the parameter to
	///
	//////////////////////////////////////////////////////////////////////////////
	void parameter( GLenum pname, const GLfloat* param ) {
		bind();
		check_gl_error( glTexParameterfv( Target, pname, param ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set a texture parameter
	///
	/// Set the given parameter to the given value
	///
	/// \param pname Name of the parameter to set
	/// \param param Pointer to the value to set the parameter to
	///
	//////////////////////////////////////////////////////////////////////////////
	void parameter( GLenum pname, const GLint* param ) {
		bind();
		check_gl_error( glTexParameteriv( Target, pname, param ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Allocate storage for a 1D image and upload texture data
	///
	/// This marks any previously allocated image storage for deletion by the GL.
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param internal_format Internal format of the texture
	/// \param the_width Width of the texture image
	/// \param format Format of the data passed to this method
	/// \param type Type of the data passed to this method
	/// \param data Pointer to memory containing the data to upload
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void image_1d( GLint level, GLint internal_format, T the_width, GLenum format, GLenum type, const GLvoid* data ) {
		m_width = static_cast<GLsizei>( the_width );
		bind();
		check_gl_error( glTexImage1D( Target, level, internal_format, m_width, 0, format, type, data ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Allocate storage for a 2D image and upload texture data
	///
	/// This marks any previously allocated image storage for deletion by the GL.
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param internal_format Internal format of the texture
	/// \param the_width Width of the texture image
	/// \param the_height Height of the texture image
	/// \param format Format of the data passed to this method
	/// \param type Type of the data passed to this method
	/// \param data Pointer to memory containing the data to upload
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T, typename U>
	void image_2d( GLint level, GLint internal_format, T the_width, U the_height, GLenum format, GLenum type, const GLvoid* data ) {
		m_width = static_cast<GLsizei>( the_width );
		m_height = static_cast<GLsizei>( the_height );
		bind();
		check_gl_error( glTexImage2D( Target, level, internal_format, m_width, m_height, 0, format, type, data ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Allocate storage for a 3D image and upload texture data
	///
	/// This marks any previously allocated image storage for deletion by the GL.
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param internal_format Internal format of the texture
	/// \param the_width Width of the texture image
	/// \param the_height Height of the texture image
	/// \param the_depth Depth of the texture image
	/// \param format Format of the data passed to this method
	/// \param type Type of the data passed to this method
	/// \param data Pointer to memory containing the data to upload
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T, typename U, typename V>
	void image_3d( GLint level, GLint internal_format, T the_width, U the_height, V the_depth, GLenum format, GLenum type, const GLvoid* data ) {
		m_width = static_cast<GLsizei>( the_width );
		m_height = static_cast<GLsizei>( the_height );
		m_depth = static_cast<GLsizei>( the_depth );
		bind();
		check_gl_error( glTexImage3D( Target, level, internal_format, m_width, m_height, m_depth, 0, format, type, data ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Update a section of a 1D image within the texture
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param xoffset x-offset of the region to update
	/// \param the_width Width of the region to update
	/// \param format Format of the data passed to this method
	/// \param type Type of the data passed to this method
	/// \param data Pointer to memory containing the data to upload
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T, typename U>
	void sub_image_1d( GLint level, T xoffset, U the_width, GLenum format, GLenum type, const GLvoid* data ) {
		assert( xoffset + the_width < m_width );
		bind();
		check_gl_error( glTexSubImage1D( Target, level, static_cast<GLint>( xoffset ), static_cast<GLsizei>( the_width ), format, type, data ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Update a section of a 2D image within the texture
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param xoffset x-offset of the region to update
	/// \param yoffset y-offset of the region to update
	/// \param the_width Width of the region to update
	/// \param the_height Height of the region to update
	/// \param format Format of the data passed to this method
	/// \param type Type of the data passed to this method
	/// \param data Pointer to memory containing the data to upload
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T, typename U, typename V, typename W>
	void sub_image_2d( GLint level, T xoffset, U yoffset, V the_width, W the_height, GLenum format, GLenum type, const GLvoid* data ) {
		assert( xoffset + the_width < m_width );
		assert( yoffset + the_height < m_height );
		bind();
		check_gl_error( glTexSubImage2D( Target, level, static_cast<GLint>( xoffset ), static_cast<GLint>( yoffset ), static_cast<GLsizei>( the_width ), static_cast<GLsizei>( the_height ), format, type, data ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Update a section of a 3D image within the texture
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param xoffset x-offset of the region to update
	/// \param yoffset y-offset of the region to update
	/// \param zoffset z-offset of the region to update
	/// \param the_width Width of the region to update
	/// \param the_height Height of the region to update
	/// \param the_depth Depth of the region to update
	/// \param format Format of the data passed to this method
	/// \param type Type of the data passed to this method
	/// \param data Pointer to memory containing the data to upload
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T, typename U, typename V, typename W, typename X, typename Y>
	void sub_image_3d( GLint level, T xoffset, U yoffset, V zoffset, W the_width, X the_height, Y the_depth, GLenum format, GLenum type, const GLvoid* data ) {
		assert( xoffset + the_width < m_width );
		assert( yoffset + the_height < m_height );
		assert( zoffset + the_depth < m_depth );
		bind();
		check_gl_error( glTexSubImage3D( Target, level, static_cast<GLint>( xoffset ), static_cast<GLint>( yoffset ), static_cast<GLint>( zoffset ), static_cast<GLsizei>( the_width ), static_cast<GLsizei>( the_height ), static_cast<GLsizei>( the_depth ), format, type, data ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get image data from this texture
	///
	/// \param level The level of detail number, 0 is the base image, level \a n is the n-th mipmap
	/// \param format Format of the returned data
	/// \param type Type of the returned data
	/// \param img Pointer to memory to write the image data to
	///
	//////////////////////////////////////////////////////////////////////////////
	void get_image( GLint level, GLint format, GLenum type, GLvoid* img ) {
		bind();
		check_gl_error( glGetTexImage( Target, level, format, type, img ) );
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Generate mipmaps for this texture
	///
	//////////////////////////////////////////////////////////////////////////////
	void generate_mipmap() {
		bind();
		check_gl_error( glGenerateMipmap( Target ) );
		unbind();
	}

protected:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that doesn't set texture parameters
	///
	/// Currently only used internally by gls::buffertexture.
	///
	//////////////////////////////////////////////////////////////////////////////
	texture(int) {
	}

private:
	object<priv::gen_textures, priv::delete_textures> m_object;
	GLsizei m_width = 0;
	GLsizei m_height = 0;
	GLsizei m_depth = 0;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::texture
/// \ingroup objects
///
/// A gls::texture is an object that encapsulates an OpenGL texture object. Like
/// all objects in GLS, the underlying name is generated at object construction
/// and deleted at destruction. This can be retrieved with name().
///
/// Texture objects store arbitrary image data within server (GPU) memory. This
/// data can then be used by shader samplers to e.g. provide texturing to
/// rendered primitives.
///
/// Texture objects can contain multiple \a images. Each image is simply a block
/// of data with an internal format. Typically, when uploading texture data to
/// apply to a primitive, one uploads it as the level 0 image of the texture. In
/// order to reduce aliasing effects that might occur when sampling from them,
/// multiple smaller versions of the level 0 image can be created. This is what
/// is known as a mipmap chain. OpenGL can automatically generate the mipmap
/// chain for you from your level 0 data when you call the generate_mipmap()
/// method. The algorithm used is implementation specific. Alternatively, you
/// can manually upload each level yourself if you wish to.
///
/// In addition to the data, each texture object has certain parameters
/// associated with it. These can control a multitude of different things.
/// Parameters that are automatically set for you on creation of a gls::texture
/// are GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER. They are both set to
/// GL_LINEAR by default, meaning OpenGL uses bilinear filtering when sampling
/// from the texture. If you generate mipmaps, those parameters should be set to
/// take them into account. Other common parameters include those that control
/// how the texture is tiled or clamped when the sampler is passed certain
/// texture coordinates in the shader.
///
/// To use a gls::texture, simply call bind(). It will bind the gls::texture to
/// its target binding and replace any previously bound to that target. To
/// clear the binding to that target, call unbind(). In newer OpenGL, textures
/// are bound to texture units which are in turn used by the samplers within a
/// shader to access texel data within them. The number of available texture
/// units depends on the hardware.
///
/// Example usage:
/// \code
/// auto texture = gls::texture<GL_TEXTURE_2D>();
/// auto texture_data = std::vector<GLubyte>{ 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255 };
/// texture.image_2d( 0, GL_RGBA, 3, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_data.data() );
/// texture.generate_mipmap();
///
/// // Disable filtering
/// texture.parameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
/// texture.parameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
///
/// glActiveTexture( GL_TEXTURE0 );
/// texture.bind();
/// glActiveTexture( GL_TEXTURE0 + 1 );
/// ... bind some other texture ...
///
/// ... render some stuff using the textures ...
///
/// glActiveTexture( GL_TEXTURE0 );
/// texture.unbind();
/// glActiveTexture( GL_TEXTURE0 + 1 );
/// ...
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
