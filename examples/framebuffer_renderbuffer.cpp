// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <iostream>

int main() {
	sf::Window window( { 200, 200 }, "Framebuffer/Renderbuffer Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, false, false ) );
	ogl_LoadFunctions();

	// Create a texture object
	auto texture = gls::texture<GL_TEXTURE_2D>();

	// Allocate texture storage for 100x100 floating point RGBA values
	texture.image_2d( 0, GL_RGBA, 100, 100, GL_RGBA, GL_FLOAT, nullptr );

	// Create a framebuffer object
	auto framebuffer = gls::framebuffer();

	// Attach level 0 of our texture to color attachment 0
	framebuffer.attach_texture( GL_COLOR_ATTACHMENT0, texture, 0 );

	// Add a 100x100 depth renderbuffer to the framebuffer object
	framebuffer.add_renderbuffer( GL_DEPTH_ATTACHMENT, gls::renderbuffer( GL_DEPTH_COMPONENT24, 100, 100 ) );

	// Check if the framebuffer is complete
	if( !framebuffer.complete() ) {
		std::cout << "Framebuffer incomplete: 0x" << std::hex << framebuffer.status() << std::endl;
	}

	// Bind our framebuffer
	framebuffer.bind();

	// Draw stuff...

	// Unbind our framebuffer
	framebuffer.unbind();

	// Generate mipmaps if we need to
	texture.generate_mipmap();

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
