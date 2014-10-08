/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>
#include <gls/errorcheck.hpp>
#include <gls/objects/object.hpp>

namespace gls {

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL sync object
///
////////////////////////////////////////////////////////////////////////////////
class sync {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///
	//////////////////////////////////////////////////////////////////////////////
	sync() :
		m_sync( 0 )
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Destructor
	///
	//////////////////////////////////////////////////////////////////////////////
	~sync() {
		check_gl_error( glDeleteSync( m_sync ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Move constructor
	///
	/// \param other Other sync that is being moved from
	///
	//////////////////////////////////////////////////////////////////////////////
	sync( sync&& other ) :
		m_sync( 0 )
	{
		std::swap( m_sync, other.m_sync );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Move assignment operator
	///
	/// \param other Other sync that is being assigned from
	/// \return *this
	///
	//////////////////////////////////////////////////////////////////////////////
	sync& operator=( sync&& other ) {
		std::swap( m_sync, other.m_sync );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \cond
	///
	//////////////////////////////////////////////////////////////////////////////
	sync( const sync& ) = delete;
	operator=( const sync& ) = delete;
	//////////////////////////////////////////////////////////////////////////////
	/// \endcond
	///
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this sync object
	///
	/// \return OpenGL name of this sync object
	///
	//////////////////////////////////////////////////////////////////////////////
	GLsync name() const {
		return m_sync;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Insert into the command queue
	///
	/// This creates a new fence sync object and inserts it into the OpenGL
	/// command queue. Any previous sync object managed by this gls::sync object
	/// is deleted prior to the creation of the new one.
	///
	//////////////////////////////////////////////////////////////////////////////
	void insert() {
		check_gl_error( glDeleteSync( m_sync ) );
		m_sync = check_gl_error( glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Wait for the sync to expire
	///
	/// If the passed timeout value is not 0, this method will only return once
	/// the timeout has passed, the sync expires or an error occurs. In the case
	/// the sync expires, either within the timeout, or prior to this method being
	/// called, true will be returned. In all other cases false will be returned.
	///
	/// Be aware that although timeout is specified in nanoseconds, the precision
	/// is not guaranteed to be that high. This method will return once <b>at
	/// least</b> that much time has passed.
	///
	/// \param timeout Time to wait for the sync to expire in nanoseconds
	/// \return true if this object expired upon returning, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	bool wait( GLuint64 timeout ) {
		auto result = check_gl_error( glClientWaitSync( m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout ) );

		auto signaled = ( ( result == GL_ALREADY_SIGNALED ) || ( result == GL_CONDITION_SATISFIED ) );

		return signaled;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Check if the current sync object has expired
	///
	/// This object becomes expired when the GL server completes execution of all
	/// commands prior to the location where this object was inserted into the
	/// command queue. Upon expiration, the underlying sync object is not yet
	/// deleted. It is only deleted when this object is destroyed, or a new sync
	/// object is to be inserted into the OpenGL command queue.
	///
	/// \return true if this object is expired, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	bool expired() {
		return wait( 0 );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Make server execution wait for this sync to expire
	///
	/// After calling this method, the server will not receive any new commands
	/// from the client until the sync has expired.
	///
	/// Because it is required, the client command queue is automatically flushed
	/// prior to waiting.
	///
	//////////////////////////////////////////////////////////////////////////////
	void server_wait() {
		check_gl_error( glFlush() );
		check_gl_error( glWaitSync( m_sync, 0, GL_TIMEOUT_IGNORED ) );
	}

private:
	GLsync m_sync;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::sync
/// \ingroup objects
///
/// A gls::sync is an object that encapsulates an OpenGL sync object. Unlike
/// other objects in GLS, the underlying sync is \a not generated at object
/// construction. It is however, deleted at destruction. A new sync is generated
/// each time the insert() method is called.
///
/// gls::sync objects are move-only.
///
/// Sync objects are relatively simple to use. One simply creates them, inserts
/// them into the command queue and either waits or periodically checks to see
/// if they have expired. The fact that they have expired provides valuable
/// information required for proper synchronization of other OpenGL code.
///
/// Example usage:
/// \code
/// auto sync = gls::sync();
///
/// auto buffer = gls::buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>();
/// auto write_data = std::vector<char>( 256000000, 0 );
/// buffer.data( write_data.size(), write_data.data() );
///
/// ... do stuff that depends on buffer's data ...
///
/// sync.insert();
///
/// ... do some other stuff to give the commands time to complete ...
///
/// if( sync.expired() ) {
/// 	... all operations that depend on buffer have completed ...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
