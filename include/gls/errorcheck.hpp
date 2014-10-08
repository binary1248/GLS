/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <gls/headercheck.hpp>

#if defined( _WIN32 )
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#if !defined( NDEBUG )
	#define check_gl_error( call ) call; gls::do_check_gl_error( __FILENAME__, __LINE__, #call )

	#if !defined( GLS_ERROR_STREAM )
		#include <iostream>
		#define GLS_ERROR_STREAM std::cerr
	#endif

	#include <iomanip>

	namespace gls {

	inline void do_check_gl_error( const char* file, unsigned int line, const char* call ) {
		auto error = glGetError();

		if( error != GL_NO_ERROR ) {
			GLS_ERROR_STREAM << "GL Error detected at " << file << ":L" << std::dec << line << std::endl;
			GLS_ERROR_STREAM << call << std::endl;
			GLS_ERROR_STREAM << "Error: ";

			switch( error ) {
			case GL_INVALID_ENUM:
				GLS_ERROR_STREAM << "GL_INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				GLS_ERROR_STREAM << "GL_INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				GLS_ERROR_STREAM << "GL_INVALID_OPERATION";
				break;
			case GL_OUT_OF_MEMORY:
				GLS_ERROR_STREAM << "GL_OUT_OF_MEMORY";
				break;
			default:
				GLS_ERROR_STREAM << "0x" << std::setw(4) << std::setfill('0') << std::hex << error;
				break;
			}

			GLS_ERROR_STREAM << std::endl << std::endl;
		}
	}

	}
#else
	#define check_gl_error( call ) (call)
#endif
