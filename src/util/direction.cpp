
#include "direction.hpp"

Direction::Direction(field_type value)
: value(value) {}

Direction::operator field_type() const {
	return value;
}

glm::ivec3 Direction::offset(Direction direction) {
	if (direction & WEST) return {-1, 0, 0};
	if (direction & EAST) return {+1, 0, 0};
	if (direction & DOWN) return {0, -1, 0};
	if (direction & UP) return {0, +1, 0};
	if (direction & NORTH) return {0, 0, -1};
	if (direction & SOUTH) return {0, 0, +1};

	return {0, 0, 0};
}

Direction Direction::opposite(Direction direction) {
	if (direction & WEST) return EAST;
	if (direction & EAST) return WEST;
	if (direction & DOWN) return UP;
	if (direction & UP) return DOWN;
	if (direction & NORTH) return SOUTH;
	if (direction & SOUTH) return NORTH;

	return NONE;
}
