// SFML Window module
#include <SFML/Window.hpp>

// GL 3.2 core loader
#include <gl_core_3_2.h>

// GLS meta-header
#include <gls.hpp>

#include <vector>

int main() {
	sf::Window window( { 200, 200 }, "Texture Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, sf::ContextSettings::Core ) );
	ogl_LoadFunctions();

	// Create a texture object
	auto texture = gls::texture<GL_TEXTURE_2D>();

	// Upload data to the texture and generate mipmap
	auto texture_data = std::vector<GLubyte>{ 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255 };
	texture.image_2d( 0, GL_RGBA, 3, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_data.data() );
	texture.generate_mipmap();

	// Set the texture to use no filtering
	texture.parameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	texture.parameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// Create a buffer texture object
	auto buffer_texture = gls::buffertexture<GL_R32F>();

	// Upload data to the buffer texture
	auto buffer_texture_data = std::vector<float>{ 1.f, 2.f, 4.f, 8.f, 16.f, 32.f, 64.f, 128.f, 256.f };
	buffer_texture.data( buffer_texture_data.size() * sizeof( float ), buffer_texture_data.data() );

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
