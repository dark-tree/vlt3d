#pragma once

#include <include/external.hpp>
#include <include/forward.hpp>
#include <logger.hpp>

class Texture {

	private:

		int width, height;
		GLenum format, internal, type;
		GLuint tex;

	public:

		Texture( int width, int height, GLenum format = GL_RGB, GLenum internal = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, GLint interpolation = GL_NEAREST );
		~Texture();

		void resize( int width, int height );

		void bind(int unit = 0);
		GLuint id();

	public:
		
		template< typename T >
		void update( T* data ) {
			this->bind();
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, this->format, this->type, (void*) data );
		}

		int getWidth();
		int getHeight();

};

class Screen : public Texture {
	
	public:
		Screen( int width, int height );

};

