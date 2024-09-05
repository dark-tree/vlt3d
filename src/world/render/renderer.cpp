
#include "renderer.hpp"
#include "mesher.hpp"
#include "world/view.hpp"

std::atomic_int world_vertex_count = 0;
std::atomic_int world_chunk_count = 0;
std::atomic_int world_frustum_count = 0;

/*
 * ChunkBuffer
 */

WorldRenderer::ChunkBuffer::ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<VertexTerrain>& mesh, int identifier)
: pos(pos), count(mesh.size()), identifier(identifier), buffer(system, count * sizeof(VertexTerrain)) {
	buffer.write(mesh.data(), mesh.size());

	#if !defined(NDEBUG)
	std::stringstream ss {};
	ss << "Chunk {";
	ss << pos;
	ss << "}";
	std::string name = ss.str();

	buffer.setDebugName(system.device, name.c_str());
	#endif
}

void WorldRenderer::ChunkBuffer::draw(Frame& frame, CommandRecorder& recorder, Frustum& frustum) {
	glm::vec3 world_pos {pos * Chunk::size};
	world_pos -= 0.5f;

	if (frame.occlusion_query.read(identifier).get(1)) {
		if (frustum.testBox3D(world_pos, world_pos + (float) Chunk::size)) {

			recorder.beginQuery(frame.occlusion_query, identifier);
			buffer.draw(recorder);
			recorder.endQuery(frame.occlusion_query, identifier);

			world_frustum_count++;
		}
	}
}

void WorldRenderer::ChunkBuffer::dispose(RenderSystem& system) {
	system.defer([this] () {
		this->buffer.close();
		delete this;
	});
}

/*
 * WorldRenderer
 */

void WorldRenderer::eraseBuffer(glm::ivec3 pos) {
	auto it = buffers.find(pos);

	if (it != buffers.end()) {
		ChunkBuffer* buffer = buffers.extract(it).second;
		world_vertex_count -= buffer->count;
		world_chunk_count --;
		buffer->dispose(system);
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
	world.consumeUpdates([&] (WorldView&& view, bool important) {
		mesher.push(std::move(view), important);
	});

	// first upload all awaiting meshes so that the PCI has something to do
	for (ChunkBuffer* chunk : awaiting.read()) {
		chunk->buffer.upload(recorder);
	}

}

void WorldRenderer::draw(CommandRecorder& recorder, Frustum& frustum) {

	world_frustum_count = 0;
	Frame& frame = system.getFrame();

	// begin rendering chunks that did not change, to not waste time during the upload from `prepare()`
	for (auto& [pos, chunk] : buffers) {
		chunk->draw(frame, recorder, frustum);
	}

	// render all new chunks and copy them into static chunk map
	for (ChunkBuffer* chunk : awaiting.read()) {
		buffers[chunk->pos] = chunk;
		chunk->draw(frame, recorder, frustum);
	}

}

void WorldRenderer::submitChunk(glm::ivec3 pos, std::vector<VertexTerrain>& mesh) {
	auto* chunk = new ChunkBuffer(system, pos, mesh, chunk_identifier ++);
	world_vertex_count += chunk->count;
	world_chunk_count ++;

	std::lock_guard lock {submit_mutex};
	allocations.push_back(chunk->buffer.getCount());
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