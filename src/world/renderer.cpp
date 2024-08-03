
#include "renderer.hpp"

class WorldRenderView {

	private:

		bool failed_to_lock = false;
		std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;

	public:

		WorldRenderView(World& world, std::shared_ptr<Chunk>& center, uint8_t directions) {
			if (!center) {
				failed_to_lock = true;
				return;
			}

			glm::ivec3 origin = center->pos;
			chunks.emplace(origin, center);

			for (uint8_t direction : Bits::decompose(directions)) {
				glm::ivec3 key = origin + Direction::offset(direction);
				std::shared_ptr<Chunk> lock = world.getChunk(key.x, key.y, key.z).lock();

				// terrain got unloaded, we are no longer in the view distance
				if (!lock) {
					failed_to_lock = true;
					return;
				}

				chunks.emplace(key, std::move(lock));
			}

		}

		bool failed() const {
			return failed_to_lock;
		}

		uint32_t getBlock(int x, int y, int z) {
			int cx = x >> Chunk::bits;
			int cy = y >> Chunk::bits;
			int cz = z >> Chunk::bits;

			return chunks[{cx, cy, cz}]->getBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask);
		}

		void close() {
			chunks.clear();
		}

};

/*
 * ChunkBuffer
 */

WorldRenderer::ChunkBuffer::ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<Vertex3D>& mesh)
: pos(pos), buffer(system, mesh.size() * sizeof(Vertex3D)) {
	buffer.write(mesh.data(), mesh.size());
}

void WorldRenderer::ChunkBuffer::draw(CommandRecorder& recorder) {
	buffer.draw(recorder);
}

/*
 * WorldRenderer
 */

void WorldRenderer::emitCube(std::vector<Vertex3D>& mesh, float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite) {
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

void WorldRenderer::eraseBuffer(RenderSystem& system, glm::ivec3 pos) {
	auto it = buffers.find(pos);

	if (it != buffers.end()) {
		ChunkBuffer* chunk = buffers.extract(it).mapped();

		// empty buffers can just be tossed away, they don't actually hold
		// any vulkan resources and never were actually passed to the GPU
		if (chunk->buffer.empty()) {
			delete chunk;
			return;
		}

		system.defer([chunk]() {
			chunk->buffer.close();
			delete chunk;
		});
	}
}

void WorldRenderer::emitMesh(RenderSystem& system, const Atlas& atlas, World& world, std::shared_ptr<Chunk> chunk) {
	if (!chunk || chunk->empty()) {
		return;
	}

	pool.enqueue([&, chunk] () mutable {
		WorldRenderView view {world, chunk, Direction::ALL};

		// failed to lock the view, this chunk must have fallen outside the render distance
		if (view.failed()) {
			return;
		}

		std::vector<Vertex3D> mesh;
		mesh.reserve(4096);

		for (int x = 0; x < Chunk::size; x++) {
			for (int y = 0; y < Chunk::size; y++) {
				for (int z = 0; z < Chunk::size; z++) {
					uint32_t block = chunk->getBlock(x, y, z);

					if (block) {
						BakedSprite sprite = (block % 2 == 1) ? atlas.getBakedSprite("vkblob") : atlas.getBakedSprite("digital");
						float shade = std::clamp((chunk->pos.y * Chunk::size + y) / (Chunk::size * 2.0f) + 0.2f, 0.0f, 1.0f);

						glm::ivec3 wpos = chunk->pos * Chunk::size + glm::ivec3 {x, y, z};

						emitCube(
							mesh,
							wpos.x,
							wpos.y,
							wpos.z,
							shade, shade, shade,
							view.getBlock(wpos.x, wpos.y + 1, wpos.z) == 0,
							view.getBlock(wpos.x, wpos.y - 1, wpos.z) == 0,
							view.getBlock(wpos.x + 1, wpos.y, wpos.z) == 0,
							view.getBlock(wpos.x - 1, wpos.y, wpos.z) == 0,
							view.getBlock(wpos.x, wpos.y, wpos.z + 1) == 0,
							view.getBlock(wpos.x, wpos.y, wpos.z - 1) == 0,
							sprite
						);
					}
				}
			}
		}

		view.close();
		submitChunk(new ChunkBuffer(system, chunk->pos, mesh));
	});
}

void WorldRenderer::prepare(World& world, RenderSystem& system, CommandRecorder& recorder) {

	// this whole section is locked both the `unused` vector
	// and `awaiting` double buffered vector are used during submitting
	{
		std::lock_guard lock {submit_mutex};
		awaiting.swap();

		for (glm::ivec3 pos : erasures) {
			eraseBuffer(system, pos);
		}

		erasures.clear();
	}

	// iterate all chunks that were updated this frame and need to be re-meshed
	world.consumeUpdates([&] (glm::ivec3 pos) {
		if (const auto chunk = world.getChunk(pos.x, pos.y, pos.z).lock()) {
			emitMesh(system, system.assets.getAtlas(), world, chunk);
		}
	});

	// first upload all awaiting meshes so that the PCI has something to do
	for (ChunkBuffer* chunk : awaiting.read()) {
		chunk->buffer.upload(recorder);
	}

}

void WorldRenderer::draw(CommandRecorder& recorder) {

	// begin rendering chunks that did not change, to not waste time during the upload from `prepare()`
	for (auto& [pos, chunk] : buffers) {
		chunk->draw(recorder);
	}

	// render all new chunks and copy them into static chunk map
	for (ChunkBuffer* chunk : awaiting.read()) {
		buffers[chunk->pos] = chunk;
		chunk->draw(recorder);
	}

}

void WorldRenderer::submitChunk(ChunkBuffer* chunk) {
	std::lock_guard lock {submit_mutex};
	awaiting.write().push_back(chunk);
	erasures.emplace_back(chunk->pos);
}

void WorldRenderer::eraseChunk(glm::ivec3 pos) {
	std::lock_guard lock {submit_mutex};
	erasures.emplace_back(pos);
}

void WorldRenderer::eraseOutside(glm::ivec3 origin, float radius) {
	glm::vec3 viewer = {origin.x / Chunk::size, origin.y / Chunk::size, origin.z / Chunk::size};
	std::lock_guard lock {submit_mutex};

	for (auto& [pos, chunk] : buffers) {
		if (glm::length(glm::vec3(pos) - viewer) > radius) {
			erasures.emplace_back(pos);
		}
	}
}