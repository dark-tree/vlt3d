
#include "renderer.hpp"
#include "mesher.hpp"

/*
 * ChunkBuffer
 */

WorldRenderer::ChunkBuffer::ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<VertexTerrain>& mesh)
: pos(pos), block(nullptr), empty(mesh.empty()), count(mesh.size()) {
	if (!empty) {
		this->block = system.unified_buffer.write(mesh.data(), mesh.size());
	}
}

void WorldRenderer::ChunkBuffer::draw(RenderSystem& system, CommandRecorder& recorder, Frustum& frustum) {
	if (!empty) {
		glm::vec3 world_pos {pos * Chunk::size};
		world_pos -= 0.5f;

		if (frustum.testBox3D(world_pos, world_pos + (float) Chunk::size)) {
			recorder.bindBuffer(system.unified_buffer.buffer, block->getOffset() * 16);
			recorder.draw(count);
		}
	}
}

void WorldRenderer::ChunkBuffer::dispose(RenderSystem& system) {

	// empty buffers can just be tossed away, they don't actually hold
	// any vulkan resources and never were actually passed to the GPU
	if (empty) {
		delete this;
		return;
	}

	system.defer([this, &system] () {
		system.unified_buffer.free(this->block);
		delete this;
	});
}

/*
 * WorldRenderer
 */

void WorldRenderer::eraseBuffer(glm::ivec3 pos) {
	auto it = buffers.find(pos);

	if (it != buffers.end()) {
		buffers.extract(it).mapped()->dispose(system);
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
//	for (ChunkBuffer* chunk : awaiting.read()) {
//		chunk->buffer.upload(recorder);
//	}

	system.unified_buffer.upload(recorder);

}

void WorldRenderer::draw(CommandRecorder& recorder, Frustum& frustum) {

	recorder.bindBuffer(system.unified_buffer.buffer);

	// begin rendering chunks that did not change, to not waste time during the upload from `prepare()`
	recorder.insertDebugLabel("World Draw Static", {1, 1, 1});
	for (auto& [pos, chunk] : buffers) {
		chunk->draw(system, recorder, frustum);
	}

	// render all new chunks and copy them into static chunk map
	recorder.insertDebugLabel("World Draw Updated", {1, 1, 1});
	for (ChunkBuffer* chunk : awaiting.read()) {
		buffers[chunk->pos] = chunk;
		chunk->draw(system, recorder, frustum);
	}

}

void WorldRenderer::submitChunk(glm::ivec3 pos, const std::vector<VertexTerrain>& mesh) {
	auto* chunk = new ChunkBuffer(system, pos, mesh);

	std::lock_guard lock {submit_mutex};
	//allocations.push_back(chunk->buffer.getCount());
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

void WorldRenderer::close() {
	mesher.close();

	// now we have them all this is mostly superficial
	// but for correctness let's lock the mutex
	std::lock_guard lock {submit_mutex};
	awaiting.swap();

	int count = 0;

	for (auto& [pos, chunk] : buffers) {
		chunk->dispose(system);
		count ++;
	}

	for (auto& chunk : awaiting.read()) {
		chunk->dispose(system);
		count ++;
	}

	logger::debug("Deleted ", count, " chunks");

	// TODO make less cringe or remove once unused
	{
		std::sort(allocations.begin(), allocations.end());
		std::ofstream file {"allocations.csv"};

		for (int allocation : allocations) {
			file << allocation;
			file << "\n";
		}
	}

}