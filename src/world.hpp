#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define CHUNK_SIZE 32

struct Chunk {

	int x, y, z;
	uint32_t blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE] = {0};
	std::vector<Vertex3D> mesh;

	Chunk(int x, int y, int z)
	: x(x), y(y), z(z) {}

	inline uint32_t& getBlock(int x, int y, int z) {
		return blocks[(x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + (z)];
	}

};

class World {
	private:
		std::unordered_map<glm::ivec3, Chunk*> chunks;
		std::vector<Vertex3D> world_mesh;
		siv::PerlinNoise noise;
		glm::ivec3 max_c_bounds;
		glm::ivec3 min_c_bounds;

		void drawCube(float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite, std::vector<Vertex3D>& mesh) {
			if (west) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (east) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (north) {
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (south) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (up) {
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (down) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}
		}

		Chunk* getChunk(int x, int y, int z) {
			glm::ivec3 key = { x, y, z };

			if (chunks.find(key) == chunks.end()) {
				chunks[key] = new Chunk(x, y, z);
			}

			return chunks[key];
		}

		uint32_t getBlock(int x, int y, int z, Chunk* chunk) {
			int bx = x % CHUNK_SIZE;
			int by = y % CHUNK_SIZE;
			int bz = z % CHUNK_SIZE;

			return chunk->getBlock(bx, by, bz);
		}

		uint32_t getBlock(int x, int y, int z) {
			int cx = x / CHUNK_SIZE;
			int cy = y / CHUNK_SIZE;
			int cz = z / CHUNK_SIZE;

			Chunk* chunk = getChunk(cx, cy, cz);

			return getBlock(x, y, z, chunk);
		}

		void setBlock(int x, int y, int z, uint32_t block) {
			int cx = x / CHUNK_SIZE;
			int cy = y / CHUNK_SIZE;
			int cz = z / CHUNK_SIZE;

			Chunk* chunk = getChunk(cx, cy, cz);

			setBlock(x, y, z, block, chunk);
		}

		void setBlock(int x, int y, int z, uint32_t block, Chunk* chunk) {
			int bx = x % CHUNK_SIZE;
			int by = y % CHUNK_SIZE;
			int bz = z % CHUNK_SIZE;

			chunk->getBlock(bx, by, bz) = block;
		}

	public:	

		World(int seed) 
			: noise(siv::PerlinNoise(seed)), min_c_bounds(-10, -1, -10), max_c_bounds(10, 4, 10) { }

		void generateAround(glm::ivec3 pos, int c_radius) {
			glm::ivec3 c_pos = { pos.x / CHUNK_SIZE, pos.y / CHUNK_SIZE, pos.z / CHUNK_SIZE };
			for (int x = -c_radius; x < c_radius; x++) {
				for (int y = -c_radius; y < c_radius; y++) {
					for (int z = -c_radius; z < c_radius; z++) {
						if (min_c_bounds.x <= c_pos.x + x && c_pos.x + x <= max_c_bounds.x &&
							min_c_bounds.y <= c_pos.y + y && c_pos.y + y <= max_c_bounds.y &&
							min_c_bounds.z <= c_pos.z + z && c_pos.z + z <= max_c_bounds.z &&
							glm::length(glm::vec3(c_pos) + glm::vec3(x, y, z)) < c_radius) {
							generateChunk(c_pos.x + x, c_pos.y + y, c_pos.z + z);
						}
					}
				}
			}
		}

		void generateChunk(int cx, int cy, int cz) {
			const float noise_scale = 16.0f;
			const int max_height = 32;

			for (int x = 0; x < CHUNK_SIZE; x++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					int xpos = cx * CHUNK_SIZE + x;
					int ypos = cy * CHUNK_SIZE;
					int zpos = cz * CHUNK_SIZE + z;

					int height = noise.noise2D_01(xpos / noise_scale, zpos / noise_scale) * max_height - max_height * 0.5f;

					if (ypos < height) {
						Chunk* chunk = getChunk(cx, cy, cz);
						int local_height = std::min(CHUNK_SIZE, height - ypos);
						for (int y = 0; y < local_height; y++) {
							if (y + ypos < height - 2) {
								setBlock(x, y, z, 1, chunk);
							}
							else {
								setBlock(x, y, z, 2, chunk);
							}
						}
					}
				}
			}
		}

		void draw(Atlas& atlas, TaskPool& pool) {
			std::vector<std::future<int>> futures;

			for (auto& [pos, chunk] : chunks) {
				futures.push_back(pool.defer([&, chunk]() -> int {
					chunk->mesh.clear();
					for (int x = 0; x < CHUNK_SIZE; x++) {
						for (int y = 0; y < CHUNK_SIZE; y++) {
							for (int z = 0; z < CHUNK_SIZE; z++) {
								uint32_t block = chunk->getBlock(x, y, z);

								if (block) {
									BakedSprite sprite = (block % 2 == 1) ? atlas.getSprite("assets/sprites/vkblob.png") : atlas.getSprite("assets/sprites/digital.png");
									float shade = std::clamp((chunk->y * CHUNK_SIZE + y) / (CHUNK_SIZE * 2.0f) + 0.2f, 0.0f, 1.0f);

									drawCube(
										chunk->x * CHUNK_SIZE + x,
										chunk->y * CHUNK_SIZE + y,
										chunk->z * CHUNK_SIZE + z,
										shade, shade, shade,
										(y >= CHUNK_SIZE - 1) || block != getBlock(x, y + 1, z, chunk),
										(y <= 0)              || block != getBlock(x, y - 1, z, chunk),
										(x >= CHUNK_SIZE - 1) || block != getBlock(x + 1, y, z, chunk),
										(x <= 0)              || block != getBlock(x - 1, y, z, chunk),
										(z >= CHUNK_SIZE - 1) || block != getBlock(x, y, z + 1, chunk),
										(z <= 0)              || block != getBlock(x, y, z - 1, chunk),
										sprite,
										chunk->mesh
									);
								}
							}
						}
					}
					return 0;
				}));
			}
			logger::info("World::draw mesh_generation took ", Timer::of([&]() {
				for (auto& future : futures) {
					future.get();
				}
			}).milliseconds());


			logger::info("World::draw mesh_collection took ", Timer::of([&]() {
				world_mesh.clear();
				for (auto& [pos, chunk] : chunks) {
					world_mesh.insert(world_mesh.end(), chunk->mesh.begin(), chunk->mesh.end());
				}
			}).milliseconds());
		}

		void random(int count, glm::ivec3 from, glm::ivec3 to) {
			while (count-- > 0) {
				int x = rand() % (to.x - from.x) + from.x;
				int y = rand() % (to.y - from.y) + from.y;
				int z = rand() % (to.z - from.z) + from.z;

				setBlock(x, y, z, rand());
			}
		}

		std::vector<Vertex3D>& getMesh() {
			return world_mesh;
		}

		void setBounds(glm::ivec3 c_min, glm::ivec3 c_max) {
			this->min_c_bounds = c_min;
			this->max_c_bounds = c_max;
		}
};
