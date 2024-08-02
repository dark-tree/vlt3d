
#include "direction.hpp"

glm::ivec3 Direction::offset(field_type direction) {
	if (direction & WEST) return {-1, 0, 0};
	if (direction & EAST) return {+1, 0, 0};
	if (direction & DOWN) return {0, -1, 0};
	if (direction & UP) return {0, +1, 0};
	if (direction & NORTH) return {0, 0, -1};
	if (direction & SOUTH) return {0, 0, +1};

	return {0, 0, 0};
}