
#include "box.hpp"

Box2D::Box2D()
: x1(0), y1(0), x2(0), y2(0) {}

Box2D::Box2D(float x1, float y1, float x2, float y2)
: x1(x1), y1(y1), x2(x2), y2(y2) {}

float Box2D::width() const {
	return x2 - x1;
}

float Box2D::height() const {
	return y2 - y1;
}

bool Box2D::contains(float x, float y) const {
	return x > x1 && x < x2 && y > y1 && y < y2;
}

bool Box2D::contains(glm::vec2 pos) const {
	return contains(pos.x, pos.y);
}

bool Box2D::empty() const {
	return (width() == 0) && (height() == 0);
}

glm::vec2 Box2D::begin() const {
	return {x1, y1};
}

glm::vec2 Box2D::end() const {
	return {x2, y2};
}

Box2D Box2D::scale(float scalar) const {
	return {x1 * scalar, y1 * scalar, x2 * scalar, y2 * scalar};
}

Box2D Box2D::offset(float x, float y) const {
	return {x1 + x, y1 + y, x2 + x, y2 + y};
}

Box2D Box2D::offset(glm::vec2 vec) const {
	return offset(vec.x, vec.y);
}

Box2D Box2D::inset(float scalar) const {
	return {x1 + scalar, y1 + scalar, x2 - scalar, y2 - scalar};
}

Box2D Box2D::envelop(Box2D other) const {
	return Box2D {std::min(other.x1, x1), std::min(other.y1, y1), std::max(other.x2, x2), std::max(other.y2, y2)};
}

Box2D Box2D::round() const {
	return {floor(x1), floor(y1), ceil(x2), ceil(y2)};
}
