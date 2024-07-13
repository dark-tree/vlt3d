
#include "object.hpp"

float WorldObject::distance(const WorldObject& other) const {
	return distance(other.getPosition());
}

float WorldObject::distance(glm::vec3 other) const {
	return glm::distance(getPosition(), other);
}