#pragma once

#include "chunk.hpp"
#include "world.hpp"
#include "client/vertices.hpp"
#include "client/renderer.hpp"
#include "command/recorder.hpp"
#include "client/immediate.hpp"

// move this somewhere else?
template <typename T>
class DoubleBuffered {

	private:

		std::vector<T> front;
		std::vector<T> back;

	public:

		void swap() {
			front.clear();
			std::swap(front, back);
		}

		std::vector<T>& read() {
			return front;
		}

		std::vector<T>& write() {
			return back;
		}

};

class WorldRenderer {

	private:

		// the thread pool used for meshing
		TaskPool pool;

		struct ChunkBuffer {
			glm::ivec3 pos;
			BasicBuffer buffer;

			void draw(CommandRecorder& recorder) {
				buffer.draw(recorder);
			}

			ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<Vertex3D>& mesh)
			: pos(pos), buffer(system, mesh.size() * sizeof(Vertex3D)) {
				buffer.write(mesh.data(), mesh.size());
			}
		};

		static void drawCube(std::vector<Vertex3D>& mesh, float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite) {
			if (west) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
			}

			if (east) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (north) {
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
			}

			if (south) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (up) {
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
			}

			if (down) {
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
				mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
				mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
				mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
			}
		}

		void erase(glm::ivec3 pos) {
			auto it = buffers.find(pos);

			if (it != buffers.end()) {
				unused.push_back(buffers.extract(it).mapped());
			}
		}

		void generate(RenderSystem& system, const Atlas& atlas, World& world, const std::shared_ptr<Chunk> chunk) {
			pool.enqueue([&, chunk] () {
				std::vector<Vertex3D> mesh;

				if (chunk->isEmpty()) {
					goto skip; // :P
				}

				for (int x = 0; x < Chunk::size; x++) {
					for (int y = 0; y < Chunk::size; y++) {
						for (int z = 0; z < Chunk::size; z++) {
							uint32_t block = chunk->getBlock(x, y, z);

							if (block) {
								BakedSprite sprite = (block % 2 == 1) ? atlas.getBakedSprite("vkblob") : atlas.getBakedSprite("digital");
								float shade = std::clamp((chunk->cy * Chunk::size + y) / (Chunk::size * 2.0f) + 0.2f, 0.0f, 1.0f);

								drawCube(
									mesh,
									chunk->cx * Chunk::size + x,
									chunk->cy * Chunk::size + y,
									chunk->cz * Chunk::size + z,
									shade, shade, shade,
									(y >= Chunk::size - 1) || !chunk->getBlock(x, y + 1, z),
									(y <= 0) || !chunk->getBlock(x, y - 1, z),
									(x >= Chunk::size - 1) || !chunk->getBlock(x + 1, y, z),
									(x <= 0) || !chunk->getBlock(x - 1, y, z),
									(z >= Chunk::size - 1) || !chunk->getBlock(x, y, z + 1),
									(z <= 0) || !chunk->getBlock(x, y, z - 1),
									sprite
								);
							}
						}
					}
				}

				skip:
				submitChunk(new ChunkBuffer(system, {chunk->cx, chunk->cy, chunk->cz}, mesh));
			});
		}

	private:

		// makes sure nothing gets fucked when we submit
		// chunk meshes from multiple threads
		std::mutex submit_mutex;

		// this holds the meshes of chunks that did not change (at least from
		// a rendering stand point - no new mesh is yet submitted for them)
		std::unordered_map<glm::ivec3, ChunkBuffer*> buffers;

		// this holds the buffers that have the new meshes written
		// but are not yet uploaded (copied from staging to device)
		DoubleBuffered<ChunkBuffer*> awaiting;

		// this holds the buffers that are not used anymore
		// either by being replaced by a new mesh or falling outside the render distance
		std::vector<ChunkBuffer*> unused;

	public:

		void prepare(ImmediateRenderer& immediate, World& world, RenderSystem& system, CommandRecorder& recorder) {

			// this whole section is locked both the `unused` vector
			// and `awaiting` double buffered vector are used during submitting
			{
				std::lock_guard lock {submit_mutex};
				awaiting.swap();

				// cleanup, send all the now unused buffers to
				// the frame task queue to be deallocated when unused
				for (ChunkBuffer* chunk : unused) {

					// empty buffers can just be tossed away, they don't actually hold
					// any vulkan resources and never were actually passed to the GPU
					if (chunk->buffer.empty()) {
						delete chunk;
						continue;
					}

					system.defer([chunk]() {
						chunk->buffer.close();
						delete chunk;
					});
				}

				unused.clear();
			}

			// iterate all chunks that were updated this frame and need to be re-meshed
			world.consumeUpdates([&] (glm::ivec3 pos) {
				if (const auto chunk = world.getChunk(pos.x, pos.y, pos.z).lock()) {
					generate(system, system.assets.getAtlas(), world, chunk);
				}
			});

			for (auto& [pos, chunk] : buffers) {
				immediate.setTint(255, 255, 255);
				immediate.drawCircle(pos.x * 32 + 16, pos.y * 32 + 16, pos.z * 32 + 16, 0.4);
			}

			// first upload all awaiting meshes so that the PCI has something to do
			for (ChunkBuffer* chunk : awaiting.read()) {
				chunk->buffer.upload(recorder);

				immediate.setTint(0, 0, 255);
				immediate.drawCircle(chunk->pos.x * 32 + 16, chunk->pos.y * 32 + 16, chunk->pos.z * 32 + 16, 1);
			}

			// the rendering will continue in `draw()`

		}

		void draw(CommandRecorder& recorder) {

			std::lock_guard lock {submit_mutex};

			// begin rendering chunks that did not change, to not waste time during the upload from `prepare()`
			for (auto& [pos, chunk] : buffers) {
				chunk->draw(recorder);
			}

			// copy all awaiting chunks, and render them
			// TODO introduce barriers to check if they did copy in time
			for (ChunkBuffer* chunk : awaiting.read()) {
				buffers[chunk->pos] = chunk;
				chunk->draw(recorder);
			}

		}

		/**
		 * Submit a buffer, the pointer will be managed by this class
		 */
		void submitChunk(ChunkBuffer* chunk) {
			std::lock_guard lock {submit_mutex};
			awaiting.write().push_back(chunk);
			erase(chunk->pos);
		}

		/**
		 * Discard the chunk at the given coordinates
		 */
		void eraseChunk(glm::ivec3 pos) {
			std::lock_guard lock {submit_mutex};
			erase(pos);
		}

		/**
		 * Discards all chunks that are further away than the given radius
		 */
		void eraseOutside(glm::ivec3 origin, float radius) {
			glm::vec3 viewer = {origin.x / Chunk::size, origin.y / Chunk::size, origin.z / Chunk::size};
			std::vector<glm::ivec3> outside;
			std::lock_guard lock {submit_mutex};

			for (auto& [pos, chunk] : buffers) {
				if (glm::length(glm::vec3(pos) - viewer) > radius) {
					outside.push_back(pos);
				}
			}

			for (auto pos : outside) {
				erase(pos);
			}
		}

};
