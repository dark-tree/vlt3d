
#include "texture.hpp"

Texture::Texture( int width, int height, GLenum format, GLenum internal, GLenum type, GLint interpolation ) {
	this->width = width;
	this->height = height;
	this->format = format;
	this->internal = internal;
	this->type = type;

	glGenTextures(1, &tex);  

	// set texture settings
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);

	// initialize empty texture
	this->resize( width, height );
}

Texture::~Texture() {
	 glDeleteTextures(1, &tex);
}

void Texture::resize( int width, int height ) {
	glTexImage2D( GL_TEXTURE_2D, 0, this->internal, width, height, 0, this->format, this->type, nullptr );
}

void Texture::bind(int unit) {
	glActiveTexture( GL_TEXTURE0 + unit );
	glBindTexture( GL_TEXTURE_2D, this->tex );
}

GLuint Texture::id() {
	return this->tex;
}

int Texture::getWidth() {
	return this->width;
}

int Texture::getHeight() {
	return this->height;
}

Screen::Screen( int width, int height ) : Texture( width, height, GL_RGBA, GL_RGBA32F, GL_FLOAT ) {
	// noop
}

