/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <utility>

namespace gls {

////////////////////////////////////////////////////////////////////////////////
/// \brief A RAII wrapper around any OpenGL object
///
/// \tparam Generator void(GLsizei, GLuint*) function that generates the name
/// \tparam Deleter void(GLsizei, const GLuint*) function that deletes the name
///
////////////////////////////////////////////////////////////////////////////////
template<void (*Generator)(GLsizei, GLuint*), void (*Deleter)(GLsizei, const GLuint*)>
class object {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///
	//////////////////////////////////////////////////////////////////////////////
	object() {
		check_gl_error( Generator( 1, &m_name ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Default destructor
	///
	//////////////////////////////////////////////////////////////////////////////
	~object() {
		check_gl_error( Deleter( 1, &m_name ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Move constructor
	///
	/// \param other Other object that is being moved from
	///
	//////////////////////////////////////////////////////////////////////////////
	object( object&& other ) :
		m_name( 0u )
	{
		std::swap( m_name, other.m_name );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Move assignment
	///
	/// \param other Other object that is being assigned from
	/// \return *this
	///
	//////////////////////////////////////////////////////////////////////////////
	object& operator=( object&& other ) {
		std::swap( m_name, other.m_name );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \cond
	///
	//////////////////////////////////////////////////////////////////////////////
	object( const object& ) = delete;
	object& operator=( const object& ) = delete;
	//////////////////////////////////////////////////////////////////////////////
	/// \endcond
	///
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this buffer
	///
	/// \return OpenGL name of this buffer
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_name;
	}

private:
	GLuint m_name;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::object
/// \ingroup objects
///
/// A gls::object is a simple wrapper object any type of OpenGL object that has
/// functions to generate and delete names. The object name is generated when
/// the gls::object is constructed and deleted when it is destroyed. It be
/// retrieved with name().
///
/// gls::object and all the other GLS classes that use it are move-only.
///
/// The generator template parameter has the signature:
/// void(GLsizei, GLuint*)
/// and the deleter template parameter has the signature:
/// void(GLsizei, const GLuint*)
///
/// \b Note: gls::object takes as its template parameters function pointers with
/// the default calling convention for your compiler/architecture. If you want
/// to call functions with differing calling conventions, you will have to wrap
/// those in your own free functions which you can pass to gls::object.
///
/// Example usage:
/// \code
/// namespace priv {
/// 	void my_gen_buffers( GLsizei size, GLuint* name ) { glGenBuffers( size, name ); }
/// 	void my_delete_buffers( GLsizei size, GLuint* name ) { glDeleteBuffers( size, name ); }
/// }
///
/// typedef gls::object<priv::my_gen_buffers, priv::my_delete_buffers> my_buffer_object;
///
/// int main() {
/// 	... create an OpenGL context ...
///
/// 	auto a_buffer_object = my_buffer_object();
///
/// 	... some other stuff ...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
