
#include "chunk.hpp"

Direction Chunk::getNeighboursMask(int x, int y, int z) {
	Direction::mask_type directions = Direction::NONE;

	if (x == 0) directions |= Direction::WEST;
	else if (x == mask) directions |= Direction::EAST;

	if (y == 0) directions |= Direction::DOWN;
	else if (y == mask) directions |= Direction::UP;

	if (z == 0) directions |= Direction::NORTH;
	else if (z == mask) directions |= Direction::SOUTH;

	return directions;
}

Block& Chunk::ref(int x, int y, int z) {
	return (*blocks)[x + y * size + z * size * size];
}

Chunk::Chunk(glm::ivec3 pos)
: pos(pos) {}

Chunk::~Chunk() {
	std::free(blocks);
}

void Chunk::setBlock(int x, int y, int z, Block block) {
	if (!blocks) {
		blocks = (Block (*)[size * size * size]) std::calloc(size * size * size, sizeof(Block));
	}

	ref(x, y, z) = block;
}

Block Chunk::getBlock(int x, int y, int z) {
	if (!blocks) {
		return Block {0};
	}

	return ref(x, y, z);
}

bool Chunk::empty() {
	return blocks == nullptr;
}
