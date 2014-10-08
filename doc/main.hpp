////////////////////////////////////////////////////////////
/// \mainpage
///
/// \section intro Introduction
/// Welcome to the GLS documentation. Here you will find detailed information
/// about all the GLS <a href="./annotated.html">classes</a> and their member
/// functions. GLS also makes extensive use of OpenGL definitions, so having an
/// OpenGL API reference at hand when using GLS helps as well.<br/>
/// <br/>
/// GLS stands for GL Stuff and is a header-only library which you can use to
/// reduce the amount of OpenGL boilerplate code you have to write in your
/// applications. This way you can focus more of your time on writing
/// application specific code instead. It is generally recommended that you are
/// somewhat familiar with the modern OpenGL API in order to make the most use
/// out of GLS, although it is not required.<br/>
/// <br/>
/// If you are looking for the latest version of GLS, be sure to check out the
/// GitHub repository at
/// <a href="https://github.com/binary1248/GLS">github.com/binary1248/GLS</a>.
/// <br/>
/// <br/>
/// You might notice that the <gls.hpp> meta-header is included in all the
/// examples. While this is convenient, if you want to minimize the time it
/// takes to build your application, you can also selectively include only the
/// headers you make use of in each translation unit.
///
/// \section example Basic example
/// Here is a basic example that renders a moving coloured triangle, to
/// demonstrate the reduction in code that GLS aims to achieve:
///
/// \code
///
/// // SFML Window module
/// #include <SFML/Window.hpp>
/// 
/// // GL 3.2 core loader
/// #include <gl_core_3_2.h>
/// 
/// // GLS meta-header
/// #include <gls.hpp>
/// 
/// #include <vector>
/// #include <cmath>
/// 
/// int main() {
/// 	sf::Window window( { 400, 400 }, "Triangle Example", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 3, 2, false, false ) );
/// 	ogl_LoadFunctions();
/// 
/// 	// Create and compile the vertex shader
/// 	auto vertex_shader = gls::shader<GL_VERTEX_SHADER>();
/// 
/// 	if( !vertex_shader.compile( R"(
/// 		#version 150 core
/// 
/// 		in vec3 position;
/// 
/// 		void main() {
/// 			gl_Position = vec4( position, 1.0 );
/// 		}
/// 	)" ) ) {
/// 		std::cout << "Vertex shader compilation failed." << std::endl;
/// 	}
/// 
/// 	// Create and compile the fragment shader
/// 	auto fragment_shader = gls::shader<GL_FRAGMENT_SHADER>();
/// 
/// 	if( !fragment_shader.compile( R"(
/// 		#version 150 core
/// 
/// 		uniform vec4 color;
/// 		out vec4 frag_color;
/// 
/// 		void main() {
/// 			frag_color = color;
/// 		}
/// 	)" ) ) {
/// 		std::cout << "Fragment shader compilation failed." << std::endl;
/// 	}
/// 
/// 	// Create and link the program
/// 	auto program = gls::program();
/// 
/// 	if( !program.link( vertex_shader, fragment_shader ) ) {
/// 		std::cout << "Program link failed." << std::endl;
/// 	}
/// 
/// 	auto buffer = gls::buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>();
/// 
/// 	// Write our triangle vertex position data into the buffer object
/// 	auto position_data = std::vector<float>();
/// 	position_data = { -.5f, -.5f, -1.f, .5f, -.5f, -1.f, 0.f, .5f, -1.f };
/// 	buffer.data( position_data.size() * sizeof( float ), position_data.data() );
/// 
/// 	// Create a vertex array object
/// 	auto vertexarray = gls::vertexarray();
/// 
/// 	// Bind the "position" attribute to its data source
/// 	vertexarray.bind_attribute( program, "position", buffer, 3, GL_FLOAT, GL_FALSE, 0, 0 );
/// 
/// 	// An SFML clock to add a bit of variety to our triangle
/// 	auto clock = sf::Clock();
/// 
/// 	while( window.isOpen() ) {
/// 		auto event = sf::Event();
/// 
/// 		while( window.pollEvent( event ) ) {
/// 			if( event.type == sf::Event::Closed ) {
/// 				return 0;
/// 			} else if( event.type == sf::Event::Resized ) {
/// 				check_gl_error( glViewport( 0, 0, static_cast<GLsizei>( window.getSize().x ), static_cast<GLsizei>( window.getSize().y ) ) );
/// 			}
/// 		}
/// 
/// 		auto seconds = clock.getElapsedTime().asSeconds();
/// 
/// 		// Set the "color" uniform value every frame
/// 		auto red = ( std::cos( seconds / 3.f ) + 3.f ) / 4.f;
/// 		auto green = ( std::cos( seconds / 2.f ) + 3.f ) / 4.f;
/// 		auto blue = ( std::cos( seconds / 1.f ) + 3.f ) / 4.f;
/// 
/// 		program.uniform( "color", red, green, blue, 1.f );
/// 
/// 		// Vary the vertex positions every frame
/// 		auto offset_x = std::sin( seconds / 3.f ) / 3.f;
/// 		auto offset_y = std::sin( seconds / 2.f ) / 3.f;
/// 
/// 		position_data = {
/// 			-.5f + offset_x, -.5f + offset_y, -1.f,
/// 			.5f + offset_x, -.5f + offset_y, -1.f,
/// 			0.f + offset_x, .5f + offset_y, -1.f
/// 		};
/// 		buffer.data( position_data.size() * sizeof( float ), position_data.data() );
/// 
/// 		check_gl_error( glClear( GL_COLOR_BUFFER_BIT ) );
/// 
/// 		// Use the program, bind the vertex array and draw the triangle
/// 		program.use();
/// 		vertexarray.bind();
/// 		check_gl_error( glDrawArrays( GL_TRIANGLES, 0, 3 ) );
/// 		vertexarray.unbind();
/// 		program.unuse();
/// 
/// 		window.display();
/// 	}
/// }
/// \endcode
///
/// This example is available for building yourself in the examples directory
/// under the name "triangle.cpp". It makes use of
/// <a href="http://www.sfml-dev.org/">SFML</a> for window and context
/// management, so you will need to have it available to build the example.
///
////////////////////////////////////////////////////////////