
#include "renderer.hpp"
#include "mesher.hpp"

/*
 * ChunkBuffer
 */

WorldRenderer::ChunkBuffer::ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<VertexTerrain>& mesh)
: pos(pos), buffer(system, mesh.size() * sizeof(VertexTerrain)) {
	buffer.write(mesh.data(), mesh.size());
}

void WorldRenderer::ChunkBuffer::draw(CommandRecorder& recorder, Frustum& frustum) {
	if (!buffer.empty()) {
		glm::vec3 world_pos {pos * Chunk::size};
		world_pos -= 0.5f;

		if (frustum.testBox3D(world_pos, world_pos + (float) Chunk::size)) {
			buffer.draw(recorder);
		}
	}
}

/*
 * WorldRenderer
 */

void WorldRenderer::eraseBuffer(glm::ivec3 pos) {
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

WorldRenderer::WorldRenderer(RenderSystem& system, World& world)
: system(system), world(world), mesher(*this, system, world) {}

void WorldRenderer::prepare(CommandRecorder& recorder) {

	// this whole section is locked as both the `erasures` vector
	// and `awaiting` double buffered vector are used during submitting
	{
		std::lock_guard lock {submit_mutex};
		awaiting.swap();

		for (glm::ivec3 pos : erasures) {
			eraseBuffer(pos);
		}

		erasures.clear();
	}

	// iterate all chunks that were updated this frame and need to be re-meshed
	world.consumeUpdates([&] (glm::ivec3 pos, bool important) {
		mesher.push(pos, important);
	});

	// first upload all awaiting meshes so that the PCI has something to do
	for (ChunkBuffer* chunk : awaiting.read()) {
		chunk->buffer.upload(recorder);
	}

}

void WorldRenderer::draw(CommandRecorder& recorder, Frustum& frustum) {

	// begin rendering chunks that did not change, to not waste time during the upload from `prepare()`
	for (auto& [pos, chunk] : buffers) {
		chunk->draw(recorder, frustum);
	}

	// render all new chunks and copy them into static chunk map
	for (ChunkBuffer* chunk : awaiting.read()) {
		buffers[chunk->pos] = chunk;
		chunk->draw(recorder, frustum);
	}

}

void WorldRenderer::submitChunk(glm::ivec3 pos, std::vector<VertexTerrain>& mesh) {
	auto* chunk = new ChunkBuffer(system, pos, mesh);
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