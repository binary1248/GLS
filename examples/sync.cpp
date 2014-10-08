// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>
#include <chrono>
#include <string>
#include <iostream>

int main() {
	sf::Window window( { 400, 200 }, "Sync Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, false, false ) );
	ogl_LoadFunctions();

	// Create our sync object;
	auto sync = gls::sync();

	check_gl_error( glFinish() );

	// Create the buffer object
	auto buffer1 = gls::buffer<GL_ARRAY_BUFFER, GL_STATIC_DRAW>();

	// Write 256MB of random data into the buffer object
	// Change the size of the buffer if necessary
	auto write_data = std::vector<char>( 256000000, 0 );
	buffer1.data( write_data.size(), write_data.data() );

	// Insert the first fence here
	sync.insert();
	auto sync1_insertion_time = std::chrono::high_resolution_clock::now();

	if( sync.wait( 1000000000 ) ) {
		auto elapsed = std::chrono::high_resolution_clock::now() - sync1_insertion_time;
		std::cout << "Buffer 1 upload: " << std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() << "ms" << std::endl;
	}

	check_gl_error( glFinish() );

	// Copy the data into another buffer object
	auto buffer2 = gls::buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_READ>();
	buffer2.copy_sub_data( buffer1, 0, 0, buffer1.size() );

	// Insert the second fence here
	sync.insert();
	auto sync2_insertion_time = std::chrono::high_resolution_clock::now();

	if( sync.wait( 1000000000 ) ) {
		auto elapsed = std::chrono::high_resolution_clock::now() - sync2_insertion_time;
		std::cout << "Buffer 1 to 2 copy: " << std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() << "ms" << std::endl;
	}

	check_gl_error( glFinish() );

	// Modify the data in the second buffer object
	buffer2.sub_data( 0, write_data.size(), write_data.data() );

	// Insert the third fence here
	sync.insert();
	auto sync3_insertion_time = std::chrono::high_resolution_clock::now();

	if( sync.wait( 1000000000 ) ) {
		auto elapsed = std::chrono::high_resolution_clock::now() - sync3_insertion_time;
		std::cout << "Buffer 2 modify: " << std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() << "ms" << std::endl;
	}

	check_gl_error( glFinish() );

	// Read the data back from the buffer object
	auto read_data = std::vector<char>( static_cast<std::size_t>( buffer2.size() ) );
	buffer2.get_sub_data( 0, read_data.size(), read_data.data() );

	// Insert a fourth fence here
	sync.insert();
	auto sync4_insertion_time = std::chrono::high_resolution_clock::now();

	if( sync.wait( 1000000000 ) ) {
		auto elapsed = std::chrono::high_resolution_clock::now() - sync4_insertion_time;
		std::cout << "Buffer 2 download: " << std::chrono::duration_cast<std::chrono::milliseconds>( elapsed ).count() << "ms" << std::endl;
	}

	// To measure the latency between the GPU and CPU,
	// we insert fences into the command stream and
	// check how long it takes to become signalled.
	// When the application is informed that the fence
	// is signalled, it would have made a full CPU-GPU-CPU
	// round-trip and we have the round-trip time in frames.

	// Frame counter
	auto frame_count = 0;

	// Latency in frames
	auto latency = 0;

	check_gl_error( glFinish() );

	while( window.isOpen() ) {
		auto event = sf::Event();

		while( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				window.close();
			}
		}

		// Check if the fence is signalled
		if( sync.expired() ) {
			latency = frame_count;
			frame_count = 0;

			// Insert a new fence into the command stream
			sync.insert();

			auto window_title = "Sync Example - Latency: " + std::to_string( latency ) + " frames";

			window.setTitle( window_title );
		}

		frame_count++;

		check_gl_error( glClear( GL_COLOR_BUFFER_BIT ) );
		window.display();
	}
}
