cmake_minimum_required( VERSION 2.8 )

project( GLS )

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
	set( GLS_OS_WINDOWS 1 )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
	if(ANDROID)
		set( GLS_OS_ANDROID 1 )
	else()
		set( GLS_OS_LINUX 1 )
	endif()
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" )
	set( GLS_OS_FREEBSD 1 )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	if( IOS )
		set( GLS_OS_IOS 1 )
	else()
		set( GLS_OS_MACOSX 1 )
	endif()
else()
	message( FATAL_ERROR "Unsupported operating system" )
	return()
endif()

### USER INPUT ###
set( GLS_BUILD_EXAMPLES true CACHE BOOL "Build examples." )
set( GLS_BUILD_DOC false CACHE BOOL "Generate API documentation." )

# Tell CMake where to find additional find modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/" )

if( GLS_OS_WINDOWS OR GLS_OS_IOS )
	set( INSTALL_SHARE_DIR "." )
elseif( GLS_OS_LINUX OR GLS_OS_FREEBSD OR GLS_OS_MACOSX )
	set( INSTALL_SHARE_DIR "share/GLS" )
elseif( GLS_OS_ANDROID )
	set( INSTALL_SHARE_DIR "${ANDROID_NDK}/sources/gls" )
endif()

set( INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include" )

set(
	HEADERS
	"${INCLUDE_PATH}/gls.hpp"
	"${INCLUDE_PATH}/gls/errorcheck.hpp"
	"${INCLUDE_PATH}/gls/headercheck.hpp"
	"${INCLUDE_PATH}/gls/objects/buffer.hpp"
	"${INCLUDE_PATH}/gls/objects/buffertexture.hpp"
	"${INCLUDE_PATH}/gls/objects/framebuffer.hpp"
	"${INCLUDE_PATH}/gls/objects/object.hpp"
	"${INCLUDE_PATH}/gls/objects/program.hpp"
	"${INCLUDE_PATH}/gls/objects/query.hpp"
	"${INCLUDE_PATH}/gls/objects/renderbuffer.hpp"
	"${INCLUDE_PATH}/gls/objects/shader.hpp"
	"${INCLUDE_PATH}/gls/objects/sync.hpp"
	"${INCLUDE_PATH}/gls/objects/texture.hpp"
	"${INCLUDE_PATH}/gls/objects/vertexarray.hpp"
)

file( READ "${INCLUDE_PATH}/gls.hpp" GLS_HPP )

string( REGEX MATCH ".*#define GLS_MAJOR_VERSION ([0-9]+).*#define GLS_MINOR_VERSION ([0-9]+).*#define GLS_PATCH_VERSION ([0-9]+).*" GLS_HPP "${GLS_HPP}" )
string( REGEX REPLACE ".*#define GLS_MAJOR_VERSION ([0-9]+).*" "\\1" GLS_MAJOR_VERSION "${GLS_HPP}" )
string( REGEX REPLACE ".*#define GLS_MINOR_VERSION ([0-9]+).*" "\\1" GLS_MINOR_VERSION "${GLS_HPP}" )
string( REGEX REPLACE ".*#define GLS_PATCH_VERSION ([0-9]+).*" "\\1" GLS_PATCH_VERSION "${GLS_HPP}" )

add_library( dummy_target STATIC EXCLUDE_FROM_ALL ${HEADERS} )
set_target_properties( dummy_target PROPERTIES LINKER_LANGUAGE CXX )

### EXAMPLES ###
if( GLS_BUILD_EXAMPLES )
	add_subdirectory( "examples" )
endif()

### DOCUMENTATION ###
if( GLS_BUILD_DOC )
	add_subdirectory( "doc" )
endif()

### INSTALL TARGETS ###
install(
	DIRECTORY include
	DESTINATION "."
	COMPONENT development
	FILES_MATCHING PATTERN "*.hpp"
)

install(
	FILES README AUTHORS LICENSE CHANGELOG
	DESTINATION "${INSTALL_SHARE_DIR}"
)

if( NOT GLS_OS_ANDROID )
	install(
		FILES cmake/Modules/FindGLS.cmake
		DESTINATION "${INSTALL_SHARE_DIR}/cmake/Modules"
	)
endif()