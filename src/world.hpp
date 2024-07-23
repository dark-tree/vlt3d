#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define CHUNK_SIZE 32

class Chunk;

struct ChunkBuffer {
	Chunk* chunk;
	Buffer buffer;
	int size;
};

class Chunk {
	private:
		std::vector<Vertex3D> mesh;
		std::unique_ptr<Buffer> vertex_buffer;
		uint32_t blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE] = { 0 };

		// does the chunk mesh need to be regenerated
		bool dirty;
		// does the chunk have any blocks
		bool empty;
		// did generating a chunk took so long that it is not needed anymore
		bool timeout;
		std::mutex timeout_mutex;
		
	public:
		int x, y, z;
		// is the chunk currently generating (flag used by World)
		bool busy;

		Chunk(int x, int y, int z)
		: x(x), y(y), z(z), dirty(true), vertex_buffer(nullptr), empty(true), busy(false), timeout(false) { }

		inline uint32_t getBlock(int x, int y, int z) {
			return blocks[(x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + (z)];
		}

		inline void setBlock(int x, int y, int z, uint32_t block) {
			blocks[(x * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + (z)] = block;
			this->dirty = true;
			this->empty = false;
		}

		void drawCube(float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite) {
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
			this->dirty = true;
		}

		ChunkBuffer draw(Atlas& atlas, Allocator& allocator, std::function<void(std::unique_ptr<Buffer>)> dispose_buffer) {
			int buffer_size = 0;
			if (dirty) {
				mesh.clear();
				if (!empty) {
					for (int x = 0; x < CHUNK_SIZE; x++) {
						for (int y = 0; y < CHUNK_SIZE; y++) {
							for (int z = 0; z < CHUNK_SIZE; z++) {
								uint32_t block = getBlock(x, y, z);

								if (block) {
									BakedSprite sprite = (block % 2 == 1) ? atlas.getSprite("assets/sprites/vkblob.png") : atlas.getSprite("assets/sprites/digital.png");
									float shade = std::clamp((this->y * CHUNK_SIZE + y) / (CHUNK_SIZE * 2.0f) + 0.2f, 0.0f, 1.0f);

									drawCube(
										this->x * CHUNK_SIZE + x,
										this->y * CHUNK_SIZE + y,
										this->z * CHUNK_SIZE + z,
										shade, shade, shade,
										(y >= CHUNK_SIZE - 1) || block != getBlock(x, y + 1, z),
										(y <= 0) || block != getBlock(x, y - 1, z),
										(x >= CHUNK_SIZE - 1) || block != getBlock(x + 1, y, z),
										(x <= 0) || block != getBlock(x - 1, y, z),
										(z >= CHUNK_SIZE - 1) || block != getBlock(x, y, z + 1),
										(z <= 0) || block != getBlock(x, y, z - 1),
										sprite
									);
								}
							}
						}
					}

					BufferInfo buffer_builder{ mesh.size() * sizeof(Vertex3D), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
					buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
					buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

					{
						// when timeout occurs, the buffer is being cleared by the World thread
						// do not create a new buffer in this case
						// locking this section ensures that in case of the conflict during timeout, the World thread will have to wait for the buffer to be created.
						// The World thread will delete the buffer shortly after, but clear method will not be called on incomplete buffer.
						std::lock_guard<std::mutex> lock(timeout_mutex);
						if (vertex_buffer != nullptr) {
							dispose_buffer(std::move(vertex_buffer));
							vertex_buffer = nullptr;
						}
						if (timeout) {
							dirty = true;
							return ChunkBuffer(this, Buffer(), 0);
						}
						vertex_buffer = std::make_unique<Buffer>(allocator.allocateBuffer(buffer_builder));
						buffer_size = mesh.size() * sizeof(Vertex3D);

						MemoryMap map = vertex_buffer->access().map();
						map.write(mesh.data(), mesh.size() * sizeof(Vertex3D));
						map.flush();
						map.unmap();

						dirty = false;

						return ChunkBuffer(this, *vertex_buffer, buffer_size);
					}
				}
				else {
					dirty = false;
				}
			}
			if (vertex_buffer == nullptr) {
				return ChunkBuffer(this, Buffer(), 0);
			}
			else {
				return ChunkBuffer(this, *vertex_buffer, buffer_size);
			}
		}

		void close(std::function<void(std::unique_ptr<Buffer>)> dispose_buffer) {
			if (vertex_buffer != nullptr) {
				mesh.clear();
				mesh.shrink_to_fit();
				dispose_buffer(std::move(vertex_buffer));
				vertex_buffer = nullptr;
				dirty = true;
			}
		}

		bool isClosed() {
			return vertex_buffer == nullptr;
		}

		bool isDirty() {
			return dirty;
		}

		void setDirty(bool dirty) {
			this->dirty = dirty;
		}

		void setTimeout(bool timeout) {
			std::lock_guard<std::mutex> lock(timeout_mutex);
			this->timeout = timeout;
		}

		bool getTimeout() {
			{
				std::lock_guard<std::mutex> lock(timeout_mutex);
				return timeout;
			}
		}
};

class World {
	private:
		std::unordered_map<glm::ivec3, Chunk*> chunks;
		siv::PerlinNoise noise;
		glm::ivec3 max_c_bounds;
		glm::ivec3 min_c_bounds;
		std::list<ChunkBuffer> buffers;
		std::list<std::future<ChunkBuffer>> futures;
		std::list<std::unique_ptr<Buffer>> buffers_to_close;
		std::mutex buffers_to_close_mutex;

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

			chunk->setBlock(bx, by, bz, block);
		}

		void generateChunk(int cx, int cy, int cz) {
			const float noise_scale = 16.0f;
			const int max_height = 32;

			Chunk* chunk = getChunk(cx, cy, cz);

			for (int x = 0; x < CHUNK_SIZE; x++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					int xpos = cx * CHUNK_SIZE + x;
					int ypos = cy * CHUNK_SIZE;
					int zpos = cz * CHUNK_SIZE + z;

					int height = noise.noise2D_01(xpos / noise_scale, zpos / noise_scale) * max_height - max_height * 0.5f;

					if (ypos < height) {
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

		// Dispose the buffer after it is no longer needed.
		// The buffer will be deleted during the next call to closeBuffers.
		void dispose_buffer(std::unique_ptr<Buffer> buffer) {
			std::lock_guard<std::mutex> lock(buffers_to_close_mutex);
			buffers_to_close.push_back(std::move(buffer));
		}

	public:	

		World(int seed) 
			: noise(siv::PerlinNoise(seed)), min_c_bounds(-100, -1, -100), max_c_bounds(100, 4, 100) { }

		// Generate chunk data around the given position in the radius.
		// If the chunk already exists, it will not be generated again.
		// Does not generate mesh data.
		void generateAround(glm::ivec3 pos, float c_radius) {
			glm::ivec3 c_pos = { pos.x / CHUNK_SIZE, pos.y / CHUNK_SIZE, pos.z / CHUNK_SIZE };
			for (int x = -c_radius; x < c_radius; x++) {
				for (int y = -c_radius; y < c_radius; y++) {
					for (int z = -c_radius; z < c_radius; z++) {
						if (min_c_bounds.x <= c_pos.x + x && c_pos.x + x <= max_c_bounds.x &&
							min_c_bounds.y <= c_pos.y + y && c_pos.y + y <= max_c_bounds.y &&
							min_c_bounds.z <= c_pos.z + z && c_pos.z + z <= max_c_bounds.z &&
							glm::length(glm::vec3(x, y, z)) < c_radius) {
							glm::ivec3 key = { c_pos.x + x, c_pos.y + y, c_pos.z + z };
							if (chunks.find(key) == chunks.end()) {
								logger::info("Generating chunk at ", key.x, " ", key.y, " ", key.z);
								generateChunk(c_pos.x + x, c_pos.y + y, c_pos.z + z);
							}
						}
					}
				}
			}
		}

		// Generate chunk mesh around the given position in the radius.
		void draw(Atlas& atlas, TaskPool& pool, Allocator& allocator, glm::ivec3 pos, float c_radius) {
			
			glm::vec3 viewer_c_pos = { pos.x / CHUNK_SIZE, pos.y / CHUNK_SIZE, pos.z / CHUNK_SIZE };

			for (auto& [c_pos, chunk] : chunks) {
				if (glm::length(glm::vec3(c_pos) - viewer_c_pos) < c_radius) {
					if (chunk->isDirty() && chunk->busy == false) {
						chunk->busy = true;
						chunk->setTimeout(false);
						futures.push_back(pool.defer([&, chunk]() -> ChunkBuffer {
							return chunk->draw(atlas, allocator, [&](std::unique_ptr<Buffer> buffer) {
								dispose_buffer(std::move(buffer));
							});
						}));
					}
				}
				else {
					if (!chunk->isClosed()) {
						chunk->setTimeout(true);
						chunk->close([&](std::unique_ptr<Buffer> buffer) {
							dispose_buffer(std::move(buffer));
						});
						// delete buffer from buffer list
						auto it = std::find_if(buffers.begin(), buffers.end(), [&](ChunkBuffer& buffer) {
							return buffer.chunk == chunk;
						});
						if (it != buffers.end()) {
							it->chunk->setDirty(true);
							buffers.erase(it);
						}
					}
				}
			}
		}

		// Delete the unused buffers.
		void closeBuffers() {
			std::lock_guard<std::mutex> lock(buffers_to_close_mutex);
			for (auto& buffer : buffers_to_close) {
				buffer->close();
			}
			buffers_to_close.clear();
		}

		// Refresh the list of buffers that are ready to be drawn and return it.
		std::list<ChunkBuffer>& getBuffers() {	
			auto it = futures.begin();
			while (it != futures.end()) {
				auto& future = *it;
				std::future_status status = future.wait_for(std::chrono::seconds(0));
				if (status == std::future_status::ready) {
					ChunkBuffer buffer = future.get();
					if (buffer.size > 0 && buffer.chunk->getTimeout() == false) {
						buffers.push_back(buffer);
					}
					buffer.chunk->busy = false;
					it = futures.erase(it);
				}
				else {
					it++;
				}
			}
			return buffers;
		}

		// Fill the given area with random blocks.
		void random(int count, glm::ivec3 from, glm::ivec3 to) {
			while (count-- > 0) {
				int x = rand() % (to.x - from.x) + from.x;
				int y = rand() % (to.y - from.y) + from.y;
				int z = rand() % (to.z - from.z) + from.z;

				setBlock(x, y, z, rand());
			}
		}

		// Set the bounds of the world (in chunks).
		void setBounds(glm::ivec3 c_min, glm::ivec3 c_max) {
			this->min_c_bounds = c_min;
			this->max_c_bounds = c_max;
		}
};
