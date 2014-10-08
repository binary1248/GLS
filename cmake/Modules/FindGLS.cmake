# This script locates the GLS library
# ------------------------------------
#
# Usage
# -----
#
# You can enforce a specific version, one of either MAJOR.MINOR or only MAJOR.
# If nothing is specified, the version won't be checked i.e. any version will
# be accepted. GLS does not consist of multiple components, so specifying
# COMPONENTS is not required.
#
# Example:
#   find_package( GLS )     // no specific version
#   find_package( GLS 1.2 ) // version 1.2 or greater
#
# If GLS is not installed in a standard path, you can use the GLS_ROOT
# CMake (or environment) variable to tell CMake where to look for GLS.
#
# Output
# ------
#
# This script defines the following variables:
#   - GLS_FOUND:       TRUE if the GLS library is found
#   - GLS_INCLUDE_DIR: the path where GLS headers are located (the directory containing the GLS/Config.hpp file)
#
# Example:
#   find_package( GLS REQUIRED )
#   include_directories( ${GLS_INCLUDE_DIR} )
#   add_executable( myapp ... )

set(
	GLS_SEARCH_PATHS
	${GLS_ROOT}
	$ENV{GLS_ROOT}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw
	/opt/local
	/opt/csw
	/opt
)

find_path(
	GLS_INCLUDE_DIR
	gls.hpp
	PATH_SUFFIXES
		include
	PATHS
		${GLS_SEARCH_PATHS}
)

set( GLS_VERSION_OK true )
if( GLS_FIND_VERSION AND GLS_INCLUDE_DIR )
	file( READ "${GLS_INCLUDE_DIR}/gls.hpp" GLS_HPP )
	
	string( REGEX MATCH ".*#define GLS_MAJOR_VERSION ([0-9]+).*#define GLS_MINOR_VERSION ([0-9]+).*" GLS_HPP "${GLS_HPP}" )
	string( REGEX REPLACE ".*#define GLS_MAJOR_VERSION ([0-9]+).*" "\\1" GLS_VERSION_MAJOR "${GLS_HPP}" )
	string( REGEX REPLACE ".*#define GLS_MINOR_VERSION ([0-9]+).*" "\\1" GLS_VERSION_MINOR "${GLS_HPP}" )
	
	math( EXPR GLS_REQUESTED_VERSION "${GLS_FIND_VERSION_MAJOR} * 100 + ${GLS_FIND_VERSION_MINOR}" )

	if( GLS_VERSION_MAJOR OR GLS_VERSION_MINOR )
		math( EXPR GLS_VERSION "${GLS_VERSION_MAJOR} * 100 + ${GLS_VERSION_MINOR}" )

		if( GLS_VERSION LESS GLS_REQUESTED_VERSION )
			set( GLS_VERSION_OK false )
		endif()
	else()
		# GLS version is < 0.2
		if( GLS_REQUESTED_VERSION GREATER 2 )
			set( GLS_VERSION_OK false )
			set( GLS_VERSION_MAJOR 0 )
			set( GLS_VERSION_MINOR x )
		endif()
	endif()
endif()

set( GLS_FOUND true )

if( NOT GLS_INCLUDE_DIR )
	set( GLS_FOUND false )
endif()

if( NOT GLS_FOUND )
	set( FIND_GLS_ERROR "GLS not found." )
elseif( NOT GLS_VERSION_OK )
	set( FIND_GLS_ERROR "GLS found but version too low, requested: ${GLS_FIND_VERSION}, found: ${GLS_VERSION_MAJOR}.${GLS_VERSION_MINOR}" )
	set( GLS_FOUND false )
endif()

if( NOT GLS_FOUND )
	if( GLS_FIND_REQUIRED )
		message( FATAL_ERROR ${FIND_GLS_ERROR} )
	elseif( NOT GLS_FIND_QUIETLY )
		message( "${FIND_GLS_ERROR}" )
	endif()
else()
	if( GLS_FIND_VERSION )
		message( STATUS "Found GLS version ${GLS_VERSION_MAJOR}.${GLS_VERSION_MINOR} in ${GLS_INCLUDE_DIR}" )
	else()
		message( STATUS "Found GLS in ${GLS_INCLUDE_DIR}" )
	endif()
endif()
