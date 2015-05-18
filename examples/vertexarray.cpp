// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>
#include <iostream>

int main() {
	sf::Window window( { 200, 200 }, "Vertex Array Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, sf::ContextSettings::Core ) );
	ogl_LoadFunctions();

	// Create and compile the vertex shader
	auto vertex_shader = gls::shader<GL_VERTEX_SHADER>();

	if( !vertex_shader.compile( R"(
		#version 150 core

		in vec3 position;

		void main() {
			gl_Position = vec4( position, 1.0 );
		}
	)" ) ) {
		std::cout << "Vertex shader compilation failed." << std::endl;
	}

	// Create and compile the fragment shader
	auto fragment_shader = gls::shader<GL_FRAGMENT_SHADER>();

	if( !fragment_shader.compile( R"(
		#version 150 core

		out vec4 frag_color;

		void main() {
			frag_color = vec4( 1.0, 1.0, 1.0, 1.0);
		}
	)" ) ) {
		std::cout << "Fragment shader compilation failed." << std::endl;
	}

	// Create and link the program
	auto program = gls::program();

	if( !program.link( vertex_shader, fragment_shader ) ) {
		std::cout << "Program link failed." << std::endl;
	}

	auto buffer = gls::buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>();

	// Write some data into the buffer object
	auto buffer_data = std::vector<float>();
	buffer_data = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f };
	buffer.data( buffer_data.size() * sizeof( float ), buffer_data.data() );

	// Create a vertex array object
	auto vertexarray = gls::vertexarray();

	// Bind the "position" attribute to its data source
	vertexarray.bind_attribute( program, "position", buffer, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	while( window.isOpen() ) {
		auto event = sf::Event();

		while( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				return 0;
			}
		}

		check_gl_error( glClear( GL_COLOR_BUFFER_BIT ) );
		window.display();
	}
}
