
#include "raycast.hpp"

Raycast::Raycast()
: successful(false), hit(0), last(0) {}

Raycast::Raycast(glm::ivec3 hit, glm::ivec3 last)
: successful(true), hit(hit), last(last) {}

Raycast::operator bool() const {
	return successful;
}

glm::ivec3 Raycast::getPos() const {
	return hit;
}

glm::ivec3 Raycast::getTarget() const {
	return last;
}