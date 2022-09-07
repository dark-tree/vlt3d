
#include "charset.hpp"

Charset::Charset( const char* path, int size ) {

	this->tex = Resource::texture(path);

	const int w = this->tex->getWidth();
	const int h = this->tex->getHeight();

	if( w % size != 0 || h % size != 0 ) {
		logger::error( "Invalid size of font: '", path, "'" );
		throw std::runtime_error("Font loading error");
	}
  
	glm::fvec2 siz( size / float(w), size / float(h) );

	for( int y = 0; y < h; y += size ) {
		for( int x = 0; x < w; x += size ) {

			glm::fvec2 uv( x / float(w), (h-y-8) / float(h) );

			Glyph glyph;
			glyph.size = siz;
			glyph.uv = uv;

			characters.push_back( glyph );
		}
	}

}

Charset::~Charset() {
	delete this->tex;
}

Glyph& Charset::get( char index ) {
	return characters.at((int) index);
}

Texture& Charset::texture() {
	return *this->tex;
}

