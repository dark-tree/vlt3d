
#include "world.hpp"
#include "generator.hpp"
#include "util/threads.hpp"

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

	for (Direction direction : Bits::decompose(Direction::ALL)) {
		glm::ivec3 neighbour = Direction::offset(direction) + chunk;

		if (!chunks.contains(neighbour)) {
			return false;
		}
	}

	return true;
}

void World::pushChunkUpdate(glm::ivec3 chunk, uint8_t flags) {
	std::lock_guard lock {updates_mutex};
	updates[chunk] |= flags;
}

void World::update(WorldGenerator& generator, glm::ivec3 origin, float radius) {
	glm::ivec3 pos = {origin.x / Chunk::size, origin.y / Chunk::size, origin.z / Chunk::size};
	float magnitude = radius * radius;

	// chunk unloading
	for (auto it = chunks.begin(); it != chunks.end();) {
		if (glm::distance2(glm::vec3(it->first), glm::vec3(pos)) >= magnitude) {
			it = chunks.erase(it);
		} else {
			it ++;
		}
	}

	// TODO
	static TaskPool pool {8};
	static std::unordered_set<glm::ivec3> requested;
	static std::mutex mutex;

	int rings = (int) radius;
	int ring = 0;

	while (ring < rings) {
		planeRingIterator(ring, [&, rings] (int cx, int cz) {
			for (int cy = -rings; cy <= rings; cy++) {
				glm::ivec3 key = {pos.x + cx, pos.y + cy, pos.z + cz};
				std::lock_guard lock {mutex};
				auto it = chunks.find(key);

				if (glm::length2(glm::vec3(cx, cy, cz)) < magnitude) {
					if (it == chunks.end()) {

						if (requested.size() > 8 || requested.contains(key)) {
							continue;
						}

						requested.insert(key);

						pool.enqueue([this, &generator, key] () {
							Chunk* chunk = generator.get(key);

							std::lock_guard lock {mutex};
							chunks[key].reset(chunk);
							pushChunkUpdate(key, ChunkUpdate::INITIAL_LOAD);
							requested.erase(key);
						});

						continue;
					}
				}
			}
		});

		ring ++;
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

Block World::getBlock(int x, int y, int z) {
	int cx = x >> Chunk::bits;
	int cy = y >> Chunk::bits;
	int cz = z >> Chunk::bits;

	if (auto chunk = getChunk(cx, cy, cz).lock()) {
		return chunk->getBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask);
	}

	throw AccessError {x, y, z};
}

void World::setBlock(int x, int y, int z, Block block) {
	int cx = x >> Chunk::bits;
	int cy = y >> Chunk::bits;
	int cz = z >> Chunk::bits;

	if (auto chunk = getChunk(cx, cy, cz).lock()) {
		int mx = x & Chunk::mask;
		int my = y & Chunk::mask;
		int mz = z & Chunk::mask;

		// setting non-air blocks doesn't require updating neighbours (for now)
		pushChunkUpdate({cx, cy, cz}, ChunkUpdate::IMPORTANT | (block.isAir() ? Chunk::getNeighboursMask(mx, my, mz) : Direction::NONE));

		return chunk->setBlock(mx, my, mz, block);
	}

	throw AccessError {x, y, z};
}

Raycast World::raycast(glm::vec3 from, glm::vec3 direction, float distance) {

	const float magnitude = distance * distance;
	glm::vec3 diff = direction * distance;
	glm::ivec3 sign = glm::sign(diff);
	glm::ivec3 pos = glm::round(from);
	glm::vec3 inv = glm::abs(1.0f / diff);

	glm::vec3 max {
		inv.x * (sign.x > 0 ? (pos.x + (0.5f - from.x)) : (from.x - (pos.x - 0.5f))),
		inv.y * (sign.y > 0 ? (pos.y + (0.5f - from.y)) : (from.y - (pos.y - 0.5f))),
		inv.z * (sign.z > 0 ? (pos.z + (0.5f - from.z)) : (from.z - (pos.z - 0.5f)))
	};

	try {
		Block block = getBlock(pos.x, pos.y, pos.z);

		if (!block.isAir()) {
			return {pos, pos};
		}

		while (glm::length2(from - glm::vec3(pos)) <= magnitude) {
			glm::ivec3 last = pos;

			if (max.x < max.y) {
				if (max.x < max.z) {
					pos.x += sign.x;
					max.x += inv.x;
				} else {
					pos.z += sign.z;
					max.z += inv.z;
				}
			} else {
				if (max.y < max.z) {
					pos.y += sign.y;
					max.y += inv.y;
				} else {
					pos.z += sign.z;
					max.z += inv.z;
				}
			}

			block = getBlock(pos.x, pos.y, pos.z);
			if (!block.isAir()) {
				return {pos, last};
			}
		}

		// If we reach here, we've traveled the full distance without hitting anything
		return {};

	} catch (AccessError& error) {
		return {};
	}
}