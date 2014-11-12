// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>
#include <iostream>

int main() {
	sf::Window window( { 600, 600 }, "Camera Example - FPS controls - E to unlock mouse", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, false, false ) );
	ogl_LoadFunctions();

	// Create and compile the vertex shader
	auto vertex_shader = gls::shader<GL_VERTEX_SHADER>();

	if( !vertex_shader.compile( R"(
		#version 150 core

		uniform mat4 view_projection;
		in vec3 position;

		void main() {
			gl_Position = view_projection * vec4( position, 1.0 );
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
			frag_color = vec4( 1.0, 1.0, 0.0, 1.0 );
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

	// Set up our camera
	auto aspect = static_cast<float>( window.getSize().x ) / static_cast<float>( window.getSize().y );
	auto camera = gls::camera<float>( gls::camera<float>::pi * 90 / 180, aspect, .1f, 100.f );

	// Variables to store our rotation state
	auto pitch = 0.f;
	auto yaw = 0.f;

	// An SFML clock to measure the elapsed time in each frame
	auto clock = sf::Clock();

	// Lock the mouse to the middle of the window
	auto mouse_lock = true;
	sf::Mouse::setPosition( static_cast<sf::Vector2i>( window.getSize() ) / 2, window );

	while( window.isOpen() ) {
		auto event = sf::Event();

		// Check how much the mouse has moved since last frame
		const auto mouse_position = sf::Mouse::getPosition( window );
		auto mouse_delta = mouse_position - static_cast<sf::Vector2i>( window.getSize() ) / 2;

		while( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				return 0;
			} else if( event.type == sf::Event::KeyPressed ) {
				if( event.key.code == sf::Keyboard::Escape ) {
					return 0;
				} else if( event.key.code == sf::Keyboard::E ) {
					// Handle locking/unlocking of mouse
					mouse_lock = !mouse_lock;

					// This is to avoid "jumps" in rotation when re-locking the cursor
					mouse_delta = sf::Vector2i( 0, 0 );
				}
			} else if( event.type == sf::Event::Resized ) {
				check_gl_error( glViewport( 0, 0, static_cast<GLsizei>( window.getSize().x ), static_cast<GLsizei>( window.getSize().y ) ) );

				// Update the aspect ratio of our camera
				camera.set_aspect( static_cast<float>( window.getSize().x ) / static_cast<float>( window.getSize().y ) );
			}
		}

		auto elapsed_seconds = clock.restart().asSeconds();

		constexpr static auto camera_movement_speed = 1.f;

		// Accumulate the required movement for this frame
		auto forward = 0.f;
		auto up = 0.f;
		auto right = 0.f;

		// Translate keyboard input to camera movement
		if( sf::Keyboard::isKeyPressed( sf::Keyboard::W ) ) {
			forward += elapsed_seconds * camera_movement_speed;
		} else if( sf::Keyboard::isKeyPressed( sf::Keyboard::S ) ) {
			forward -= elapsed_seconds * camera_movement_speed;
		}

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::A ) ) {
			right += elapsed_seconds * camera_movement_speed;
		} else if( sf::Keyboard::isKeyPressed( sf::Keyboard::D ) ) {
			right -= elapsed_seconds * camera_movement_speed;
		}

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Space ) ) {
			up += elapsed_seconds * camera_movement_speed;
		} else if( sf::Keyboard::isKeyPressed( sf::Keyboard::LShift ) ) {
			up -= elapsed_seconds * camera_movement_speed;
		}

		// Move the camera relative to its own coordinate system
		camera.move_relative( { right, 0, -forward } );

		// Move the camera relative to the global coordinate system
		camera.move( { 0, up, 0 } );

		// Mouse cursor management
		if( mouse_lock ) {
			sf::Mouse::setPosition( static_cast<sf::Vector2i>( window.getSize() ) / 2, window );
		} else {
			mouse_delta = sf::Vector2i( 0, 0 );
		}

		constexpr static auto camera_rotation_speed = .01f;

		// Update the yaw and pitch values according to the mouse input
		yaw += static_cast<float>( mouse_delta.x ) * camera_rotation_speed;
		pitch += static_cast<float>( mouse_delta.y ) * camera_rotation_speed;

		// Set the new orientation based on yaw and pitch
		camera.set_orientation_taitbryan( yaw, pitch, 0.f );

		// Update the program uniform with the camera matrix
		program.uniform_matrix4( "view_projection", 1, GL_FALSE, camera.get_matrix().data() );

		check_gl_error( glClear( GL_COLOR_BUFFER_BIT ) );

		// Use the program, bind the vertex array and draw the triangle
		program.use();
		vertexarray.bind();
		check_gl_error( glDrawArrays( GL_TRIANGLES, 0, 3 ) );
		vertexarray.unbind();
		program.unuse();

		window.display();
	}
}
