
#include "chunk.hpp"

uint32_t& Chunk::ref(int x, int y, int z) {
	return (*blocks)[x + y * size + z * size * size];
}

Chunk::Chunk(glm::ivec3 pos)
: pos(pos) {}

Chunk::~Chunk() {
	delete[] blocks;
}

void Chunk::setBlock(int x, int y, int z, uint32_t block) {
	if (!blocks) {
		blocks = (uint32_t (*)[size * size * size]) std::calloc(size * size * size, sizeof(uint32_t));
	}

	ref(x, y, z) = block;
}

uint32_t Chunk::getBlock(int x, int y, int z) {
	if (!blocks) {
		return 0;
	}

	return ref(x, y, z);
}

bool Chunk::empty() {
	return blocks == nullptr;
}
