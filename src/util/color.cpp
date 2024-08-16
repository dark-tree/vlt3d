
#include "color.hpp"

Color::Color()
: Color(255, 255, 255) {}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
: r(r), g(g), b(b), a(a) {}

glm::vec4 Color::toFloat() const {
	return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}