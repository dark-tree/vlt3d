#pragma once

#include <include/external.hpp>
#include <logger.hpp>
#include <renderer/texture.hpp>
#include <loader/resource.hpp>

struct Glyph {

	// Offset to advance to next glyph
	//GLuint advance;

	// Size of glyph
	glm::fvec2 size;

	// Texture uv
	glm::fvec2 uv;

	// Offset from baseline to left/top of glyph
	//glm::ivec2 bearing;

};

class Charset {

	private:
		std::vector<Glyph> characters;
		Texture* tex;

	public:
		Charset( const char* path, int size = 8  );
		~Charset();

		Glyph& get( char );
		Texture& texture();

};


