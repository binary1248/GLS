cmake_minimum_required( VERSION 2.8 )

# Tell CMake where to find additional find modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/" )

set( SFML_STATIC_LIBRARIES false CACHE BOOL "Was SFML built as a static library?" )

# Find packages.
find_package( OpenGL REQUIRED )
find_package( SFML 2 REQUIRED COMPONENTS window system )

# Find X11 for glX on Linux, checking for UNIX would match other UNIX systems as well
if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
	find_package( X11 REQUIRED )
endif()

set( INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include" )
set( GL_LOADER_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/examples/gl_core_3_2" )

include_directories( "${INCLUDE_PATH}" )
include_directories( SYSTEM "${GL_LOADER_INCLUDE_PATH}" )
include_directories( SYSTEM "${SFML_INCLUDE_DIR}" )

if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
	set( CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wunused-parameter -Wno-long-long -pedantic ${CMAKE_CXX_FLAGS}" )
	set( CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Wno-long-long -pedantic ${CMAKE_C_FLAGS}" )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	set( CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wno-long-long -pedantic ${CMAKE_CXX_FLAGS}" )
	set( CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Wno-long-long -pedantic ${CMAKE_C_FLAGS}" )
endif()

function( build_example EXAMPLE_NAME SOURCES )
	add_executable( ${EXAMPLE_NAME} ${SOURCES} "gl_core_3_2/gl_core_3_2.c" )
	set_target_properties( ${EXAMPLE_NAME} PROPERTIES DEBUG_POSTFIX -d )
	target_link_libraries( ${EXAMPLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${OPENGL_gl_LIBRARY} )
endfunction()

build_example( "buffer" "buffer.cpp" )
build_example( "camera" "camera.cpp" )
build_example( "framebuffer_renderbuffer" "framebuffer_renderbuffer.cpp" )
build_example( "query" "query.cpp" )
build_example( "shader_program" "shader_program.cpp" )
build_example( "sync" "sync.cpp" )
build_example( "texture" "texture.cpp" )
build_example( "triangle" "triangle.cpp" )
build_example( "vertexarray" "vertexarray.cpp" )
