#pragma once

#include "external.hpp"
#include "chunk.hpp"
#include "util/logger.hpp"
#include "util/direction.hpp"
#include "util/bits.hpp"

struct AccessError {

	const int x, y, z;

	AccessError(int x, int y, int z)
	: x(x), y(y), z(z) {}

};

class World {

	private:

		siv::PerlinNoise noise;
		std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;
		std::unordered_map<glm::ivec3, uint8_t> updates;

		Chunk* generateChunk(int cx, int cy, int cz) {
			const float noise_scale = 16.0f;
			const int max_height = 32;

			Chunk* chunk = new Chunk({cx, cy, cz});

			for (int x = 0; x < Chunk::size; x++) {
				for (int z = 0; z < Chunk::size; z++) {
					int xpos = cx * Chunk::size + x;
					int ypos = cy * Chunk::size;
					int zpos = cz * Chunk::size + z;

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

		bool isChunkRenderReady(glm::ivec3 chunk) const {
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

	public:

		template <typename Func>
		void consumeUpdates(Func func) {

			std::unordered_set<glm::ivec3> set;
			set.reserve(updates.size() * 2);

			// propagate updates
			for (auto& [pos, directions] : updates) {
				for (uint8_t direction : Bits::decompose(directions)) {
					glm::ivec3 neighbour = Direction::offset(direction) + pos;

					if (isChunkRenderReady(neighbour)) {
						set.insert(neighbour);
					}
				}

				if (isChunkRenderReady(pos)) {
					set.insert(pos);
				}
			}

			updates.clear();

			// call once for each updated chunk
			for (auto pos : set) {
				func(pos);
			}
		}

		void pushChunkUpdate(glm::ivec3 chunk, uint8_t directions) {
			updates[chunk] |= directions;
		}

		/**
		 * Update the world, manages chunk loading and unloading
		 */
		void update(glm::ivec3 origin, float radius) {
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
//							if (awaiting.contains(key)) {
//								continue;
//							}

							if (it == chunks.end()) {
//								awaiting.insert(key);
								chunks[key].reset(generateChunk(key.x, key.y, key.z));
								pushChunkUpdate(key, 0b00111111);

								continue;
							}
						}
					}
				}
			}
		}

		std::weak_ptr<Chunk> getChunk(int cx, int cy, int cz) {
			const glm::ivec3 key {cx, cy, cz};
			auto it = chunks.find(key);

			if (it == chunks.end()) {
				return {};
			}

			return it->second;
		}

		uint32_t getBlock(int x, int y, int z) {
			int cx = x >> Chunk::bits;
			int cy = y >> Chunk::bits;
			int cz = z >> Chunk::bits;

			if (auto chunk = getChunk(cx, cy, cz).lock()) {
				return chunk->getBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask);
			}

			throw AccessError {x, y, z};
		}

		void setBlock(int x, int y, int z, uint32_t block) {
			int cx = x >> Chunk::bits;
			int cy = y >> Chunk::bits;
			int cz = z >> Chunk::bits;

			if (auto chunk = getChunk(cx, cy, cz).lock()) {
				return chunk->setBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask, block);
			}

			throw AccessError {x, y, z};
		}

};
