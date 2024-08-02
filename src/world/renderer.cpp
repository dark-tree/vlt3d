
#include "renderer.hpp"

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

void WorldRenderer::erase(glm::ivec3 pos) {
	auto it = buffers.find(pos);

	if (it != buffers.end()) {
		unused.push_back(buffers.extract(it).mapped());
	}
}

void WorldRenderer::emitMesh(RenderSystem& system, const Atlas& atlas, World& world, std::shared_ptr<Chunk> chunk) {
	pool.enqueue([&, chunk] () {
		std::vector<Vertex3D> mesh;

		if (chunk->empty()) {
			goto skip; // :P
		}

		for (int x = 0; x < Chunk::size; x++) {
			for (int y = 0; y < Chunk::size; y++) {
				for (int z = 0; z < Chunk::size; z++) {
					uint32_t block = chunk->getBlock(x, y, z);

					if (block) {
						BakedSprite sprite = (block % 2 == 1) ? atlas.getBakedSprite("vkblob") : atlas.getBakedSprite("digital");
						float shade = std::clamp((chunk->pos.y * Chunk::size + y) / (Chunk::size * 2.0f) + 0.2f, 0.0f, 1.0f);

						emitCube(
							mesh,
							chunk->pos.x * Chunk::size + x,
							chunk->pos.y * Chunk::size + y,
							chunk->pos.z * Chunk::size + z,
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
		submitChunk(new ChunkBuffer(system, chunk->pos, mesh));
	});
}

void WorldRenderer::prepare(ImmediateRenderer& immediate, World& world, RenderSystem& system, CommandRecorder& recorder) {

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
			emitMesh(system, system.assets.getAtlas(), world, chunk);
		}
	});

//	// uncomment to see empty chunks
//	for (auto& [pos, chunk] : buffers) {
//		immediate.setTint(255, 255, 255);
//		immediate.drawCircle(pos.x * 32 + 16, pos.y * 32 + 16, pos.z * 32 + 16, 0.4);
//	}

	// first upload all awaiting meshes so that the PCI has something to do
	for (ChunkBuffer* chunk : awaiting.read()) {
		chunk->buffer.upload(recorder);

//		// uncomment to see "to be uploaded" chunks
//		immediate.setTint(0, 0, 255);
//		immediate.drawCircle(chunk->pos.x * 32 + 16, chunk->pos.y * 32 + 16, chunk->pos.z * 32 + 16, 1);
	}

}

void WorldRenderer::draw(CommandRecorder& recorder) {

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

void WorldRenderer::submitChunk(ChunkBuffer* chunk) {
	std::lock_guard lock {submit_mutex};
	awaiting.write().push_back(chunk);
	erase(chunk->pos);
}

void WorldRenderer::eraseChunk(glm::ivec3 pos) {
	std::lock_guard lock {submit_mutex};
	erase(pos);
}

void WorldRenderer::eraseOutside(glm::ivec3 origin, float radius) {
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