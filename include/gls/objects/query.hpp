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
	void gen_queries( GLsizei size, GLuint* name ) { glGenQueries( size, name ); }
	void delete_queries( GLsizei size, const GLuint* name ) { glDeleteQueries( size, name ); }
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating an OpenGL query object
///
/// \tparam Target The target established by this query when it is active
///
////////////////////////////////////////////////////////////////////////////////
template<GLenum Target>
class query {
public:
	//////////////////////////////////////////////////////////////////////////////
	/// \brief Retrieve the OpenGL name of this query
	///
	/// \return OpenGL name of this query
	///
	//////////////////////////////////////////////////////////////////////////////
	GLuint name() const {
		return m_object.name();
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Run the query on the provided callable
	///
	/// The callable should take no parameters, and its return value is ignored by
	/// run().
	///
	/// If a previous query is not pending begin() will be called, followed by the
	/// callable and then end(). Prefer using this method over begin() and end()
	/// when possible.
	///
	/// \param callable Callable to run between begin() and end()
	///
	//////////////////////////////////////////////////////////////////////////////
	template<typename T>
	void run( T callable ) {
		if( m_waiting ) {
			callable();
			return;
		}

		m_waiting = true;

		check_gl_error( glBeginQuery( Target, m_object.name() ) );
		callable();
		check_gl_error( glEndQuery( Target ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Mark the begin of the query block
	///
	/// If a previous query is still pending, calling this method will have no
	/// affect.
	///
	//////////////////////////////////////////////////////////////////////////////
	void begin() {
		if( m_waiting ) {
			return;
		}

		m_waiting = true;

		check_gl_error( glBeginQuery( Target, m_object.name() ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Mark the end of the query block
	///
	/// Calling this method if this query is not currently active will produce an
	/// OpenGL error.
	///
	//////////////////////////////////////////////////////////////////////////////
	void end() {
		check_gl_error( glEndQuery( Target ) );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Poll the result of the query
	///
	/// After running run() or ending a block with end(), you can periodically
	/// poll the result of the query using this method. It will return true to
	/// signal the result passed in the result parameter is valid for use and
	/// false otherwise if the query is not yet ready.
	///
	/// \param result Variable to store the query result in
	/// \return true if the result is ready and valid, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	bool poll_result( GLint& result ) {
		if( m_waiting && !ready() ) {
			return false;
		}

		check_gl_error( glGetQueryObjectiv( name(), GL_QUERY_RESULT, &result ) );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Poll the result of the query
	///
	/// After running run() or ending a block with end(), you can periodically
	/// poll the result of the query using this method. It will return true to
	/// signal the result passed in the result parameter is valid for use and
	/// false otherwise if the query is not yet ready.
	///
	/// \param result Variable to store the query result in
	/// \return true if the result is ready and valid, false otherwise
	///
	//////////////////////////////////////////////////////////////////////////////
	bool poll_result( GLuint& result ) {
		if( m_waiting && !ready() ) {
			return false;
		}

		check_gl_error( glGetQueryObjectuiv( name(), GL_QUERY_RESULT, &result ) );

		return true;
	}

private:
	bool ready() {
		auto result_ready = GLuint();

		check_gl_error( glGetQueryObjectuiv( name(), GL_QUERY_RESULT_AVAILABLE, &result_ready ) );

		m_waiting = ( result_ready == GL_FALSE );

		return result_ready == GL_TRUE;
	}

	object<priv::gen_queries, priv::delete_queries> m_object;
	bool m_waiting = false;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::query
/// \ingroup objects
///
/// A gls::query is an object that encapsulates an OpenGL query object. Like
/// all objects in GLS, the underlying name is generated at object construction
/// and deleted at destruction. This can be retrieved with name().
///
/// Query objects are used to query counters in the GL server. The counters are
/// reset when the query begins and their value stored in the query object once
/// the query ends. Common targets for a query are GL_SAMPLES_PASSED and
/// GL_PRIMITIVES_GENERATED. If ARB_timer_query or OpenGL 3.3 is available,
/// GL_TIME_ELAPSED is also common.
///
/// You can use a gls::query in 2 ways. The first method is closer to the way
/// you would perform them yourself using OpenGL code directly, manually using
/// begin() and end(). The second method using run() automatically takes care of
/// calling end() for you once the callable has returned, thus it might be more
/// convenient depending on the scenario. Either way, once end() has been
/// called, either manually or automatically, you will need to periodically poll
/// for the result. Once it is ready, you can start another query using this
/// object.
///
/// Note that running 2 simultaneous queries with the same target will produce
/// an OpenGL error. This will not happen if using run().
///
/// Example usage:
/// \code
/// auto query = gls::query<GL_SAMPLES_PASSED>();
/// query.run( [&]() {
///
/// 	... some OpenGL code producing samples here ...
///
/// } );
///
/// ...
///
/// auto result = GLuint( 0 );
/// if( query.poll_result( result ) ) {
/// 	... do something with the result ...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
