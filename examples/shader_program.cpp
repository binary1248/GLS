// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>
#include <iostream>

int main() {
	sf::Window window( { 200, 200 }, "Shader/Program Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, false, false ) );
	ogl_LoadFunctions();

	// Create and compile a vertex shader
	auto vertex_shader = gls::shader<GL_VERTEX_SHADER>();

	if( !vertex_shader.compile( R"(
		#version 150 core

		in vec3 position;

		void main() {
			gl_Position = vec4( position, 1.0 );
		}
	)" ) ) {
		std::cout << "Vertex shader compilation failed." << std::endl;
		std::cout << vertex_shader.get_info_log() << std::endl;
	}

	// Create and compile a fragment shader
	auto fragment_shader = gls::shader<GL_FRAGMENT_SHADER>();

	if( !fragment_shader.compile( R"(
		#version 150 core

		uniform vec4 color;
		out vec4 frag_color;

		void main() {
			frag_color = color;
		}
	)" ) ) {
		std::cout << "Fragment shader compilation failed." << std::endl;
		std::cout << fragment_shader.get_info_log() << std::endl;
	}

	// Create and link a program
	auto program = gls::program();

  if( !program.link( vertex_shader, fragment_shader ) ) {
		std::cout << "Program link failed." << std::endl;
		std::cout << program.get_info_log() << std::endl;
  }

  {
		// Grab the location of our "position" attribute
		auto position_location = program.get_attribute_location( "position" );
		if( position_location < 0 ) {
			std::cout << "Could not find location of attribute \"position\" in the program." << std::endl;
		} else {
			std::cout << "Attribute \"position\" is located at " << position_location << "." << std::endl;
		}
  }

  {
		// Grab the location of our "color" uniform
		auto color_location = program.get_uniform_location( "color" );
		if( color_location < 0 ) {
			std::cout << "Could not find location of uniform \"color\" in the program." << std::endl;
		} else {
			std::cout << "Uniform \"color\" is located at " << color_location << "." << std::endl;
		}
  }

  {
		// Grab the binding of a "non_existant" uniform block
		auto non_existant_block_binding = program.get_uniform_block_binding( "non_existant" );
		if( non_existant_block_binding == program.no_block_binding() ) {
			std::cout << "Could not get binding of uniform block \"non_existant\" in the program." << std::endl;
		} else {
			std::cout << "Binding for uniform block \"non_existant\" is " << non_existant_block_binding << "." << std::endl;
		}
  }

  // Use the program to render stuff
  program.use();

  // Stuff...

  // Unuse the program when needed
  program.unuse();

	while( window.isOpen() ) {
		auto event = sf::Event();

		while( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				window.close();
			}
		}

		check_gl_error( glClear( GL_COLOR_BUFFER_BIT ) );
		window.display();
	}
}
