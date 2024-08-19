
#include "color.hpp"

glm::vec4 Color::toFloat() const {
	return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}