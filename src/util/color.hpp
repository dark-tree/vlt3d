#pragma once

#include "external.hpp"

class Color {

	public:

		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;

	public:

		Color();
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

		glm::vec4 toFloat() const;

};

static_assert(sizeof(Color) <= sizeof(uint8_t) * 4);