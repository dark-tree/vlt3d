
#include "world.hpp"

Chunk* World::generate(glm::ivec3 pos) const {
	const float noise_scale = 16.0f;
	const int max_height = 32;

	Chunk* chunk = new Chunk(pos);

	for (int x = 0; x < Chunk::size; x++) {
		for (int z = 0; z < Chunk::size; z++) {
			int xpos = pos.x * Chunk::size + x;
			int ypos = pos.y * Chunk::size;
			int zpos = pos.z * Chunk::size + z;

			int height = noise.noise2D_01(xpos / noise_scale, zpos / noise_scale) * max_height - max_height * 0.5f;

			if (ypos < height) {
				int local_height = std::min(Chunk::size, height - ypos);
				for (int y = 0; y < local_height; y++) {

					if (y + ypos < height - 2) {
						chunk->setBlock(x, y, z, 1);
					} else {
						chunk->setBlock(x, y, z, 2);
					}

				}
			}
		}
	}

	return chunk;
}

bool World::isChunkRenderReady(glm::ivec3 chunk) const {
	if (!chunks.contains(chunk)) {
		return false;
	}

	for (uint8_t direction : Bits::decompose(Direction::ALL)) {
		glm::ivec3 neighbour = Direction::offset(direction) + chunk;

		if (!chunks.contains(neighbour)) {
			return false;
		}
	}

	return true;
}

void World::pushChunkUpdate(glm::ivec3 chunk, uint8_t directions) {
	updates[chunk] |= directions;
}

void World::update(glm::ivec3 origin, float radius) {
	glm::ivec3 pos = {origin.x / Chunk::size, origin.y / Chunk::size, origin.z / Chunk::size};

	// chunk unloading
	for (auto it = chunks.begin(); it != chunks.end();) {
		if (glm::distance(glm::vec3(it->first), glm::vec3(pos)) >= radius) {
			it = chunks.erase(it);
		} else {
			it ++;
		}
	}

	// chunk loading
	for (int cx = -radius; cx <= radius; cx++) {
		for (int cy = -radius; cy <= radius; cy++) {
			for (int cz = -radius; cz <= radius; cz++) {

				glm::ivec3 key = {pos.x + cx, pos.y + cy, pos.z + cz};
				auto it = chunks.find(key);

				if (glm::length(glm::vec3(cx, cy, cz)) < radius) {
					if (it == chunks.end()) {
						chunks[key].reset(generate(key));
						pushChunkUpdate(key, 0b00111111);

						continue;
					}
				}
			}
		}
	}
}

std::weak_ptr<Chunk> World::getChunk(int cx, int cy, int cz) {
	const glm::ivec3 key {cx, cy, cz};
	auto it = chunks.find(key);

	if (it == chunks.end()) {
		return {};
	}

	return it->second;
}

uint32_t World::getBlock(int x, int y, int z) {
	int cx = x >> Chunk::bits;
	int cy = y >> Chunk::bits;
	int cz = z >> Chunk::bits;

	if (auto chunk = getChunk(cx, cy, cz).lock()) {
		return chunk->getBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask);
	}

	throw AccessError {x, y, z};
}

void World::setBlock(int x, int y, int z, uint32_t block) {
	int cx = x >> Chunk::bits;
	int cy = y >> Chunk::bits;
	int cz = z >> Chunk::bits;

	if (auto chunk = getChunk(cx, cy, cz).lock()) {
		return chunk->setBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask, block);
	}

	throw AccessError {x, y, z};
}
