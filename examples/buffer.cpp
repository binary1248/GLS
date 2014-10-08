// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>
#include <iostream>

int main() {
	sf::Window window( { 200, 200 }, "Buffer Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, false, false ) );
	ogl_LoadFunctions();

	// Create the buffer object
	auto buffer1 = gls::buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>();

	// Write some initial data into the buffer object
	auto write_data = std::vector<float>();
	write_data = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f };
	buffer1.data( write_data.size() * sizeof( float ), write_data.data() );

	// Copy the data into another buffer object
	auto buffer2 = gls::buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_READ>();
	buffer2.copy_sub_data( buffer1, 0, 0, buffer1.size() );

	// Modify the data in the second buffer object
	write_data = { 0.f, 0.f, 0.f };
	buffer2.sub_data( 3 * sizeof( float ), write_data.size() * sizeof( float ), write_data.data() );

	// Read the data back from the buffer object
	auto read_data = std::vector<float>();
	read_data.resize( static_cast<std::size_t>( buffer2.size() ) / sizeof( float ) );
	buffer2.get_sub_data( 0, read_data.size() * sizeof( float ), read_data.data() );

	// Print out the data
	for( auto value : read_data ) {
		std::cout << value << " ";
	}
	std::cout << std::endl;

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
