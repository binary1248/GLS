// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>
#include <string>
#include <iostream>

int main() {
	sf::Window window( { 400, 400 }, "Query Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, sf::ContextSettings::Core ) );
	window.setFramerateLimit( 60 );
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
			frag_color = vec4( 1.0, 1.0, 1.0, 1.0 );
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

	// Write our triangle vertex position data into the buffer object
	auto position_data = std::vector<float>();
	position_data = { -.5f, -.5f, -1.f, .5f, -.5f, -1.f, 0.f, .5f, -1.f };
	buffer.data( position_data.size() * sizeof( float ), position_data.data() );

	// Create a vertex array object
	auto vertexarray = gls::vertexarray();

	// Bind the "position" attribute to its data source
	vertexarray.bind_attribute( program, "position", buffer, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	// Create a query to check samples passed
	auto samples_passed_query = gls::query<GL_SAMPLES_PASSED>();

	// Create a query to check primitives generated
	auto primitives_generated_query = gls::query<GL_PRIMITIVES_GENERATED>();

	// We will use a lambda to encapsulate our drawing
	auto draw = [&]() {
		// Use the program, bind the vertex array and draw the triangle
		program.use();
		vertexarray.bind();
		check_gl_error( glDrawArrays( GL_TRIANGLES, 0, 3 ) );
		vertexarray.unbind();
		program.unuse();
	};

	// We need to create a second lambda to encapsulate the first query
	auto query_samples_passed = [&]() {
		samples_passed_query.run( draw );
	};

	// Variables to store our query results
	auto samples_passed_result = GLuint( 0 );
	auto primitives_generated_result = GLuint( 0 );

	while( window.isOpen() ) {
		auto event = sf::Event();

		while( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				return 0;
			} else if( event.type == sf::Event::Resized ) {
				check_gl_error( glViewport( 0, 0, static_cast<GLsizei>( window.getSize().x ), static_cast<GLsizei>( window.getSize().y ) ) );
			}
		}

		check_gl_error( glClear( GL_COLOR_BUFFER_BIT ) );

		// Query the primitives generated and the samples passed
		primitives_generated_query.run( query_samples_passed );

		// Poll for the results of our query
		auto samples_passed_ready = samples_passed_query.poll_result( samples_passed_result );
		auto primitives_generated_ready = primitives_generated_query.poll_result( primitives_generated_result );

		// If any was ready, use the window title to output them
		if( samples_passed_ready || primitives_generated_ready ) {
			auto window_title = std::string( "Query Example - " );
			window_title += std::to_string( primitives_generated_result ) + " primitives - ";
			window_title += std::to_string( samples_passed_result ) + " samples";
			window.setTitle( window_title );
		}

		window.display();
	}
}
