
#include "world.hpp"
#include "generator.hpp"

/*
 * AccessError
 */

const char* AccessError::what() const noexcept {
	return "World Access Error";
}

/*
 * World
 */

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

void World::update(WorldGenerator& generator, glm::ivec3 origin, float radius) {
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
						chunks[key].reset(generator.get(key));
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

		// TODO this can be more optimal ofc
		//   1) don't push an update if the block did not change
		//   2) setting non-air blocks doesn't require updating neighbours
		//   3) when neighbours are to be updated only update the ones touching the block (possibly none)
		pushChunkUpdate({cx, cy, cz}, Direction::ALL);

		return chunk->setBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask, block);
	}

	throw AccessError {x, y, z};
}
