
#include "renderer.hpp"
#include "mesher.hpp"
#include "world/view.hpp"

// TODO TODO TODO TODO
std::atomic_int world_vertex_count = 0;
std::atomic_int world_chunk_count = 0;
std::atomic_int world_frustum_count = 0;
std::atomic_int world_occlusion_count = 0;

/*
 * ChunkBuffer
 */

WorldRenderer::ChunkBuffer::ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<VertexTerrain>& mesh)
: pos(pos), count(mesh.size()), identifier(system.predicate_allocator.allocate()), buffer(system, count * sizeof(VertexTerrain)) {
	buffer.write(mesh.data(), mesh.size());

	if (identifier == LinearArena::failed) {
		throw Exception {"Out of unique chunk occlusion identifiers! Tell magistermaks to fix it!"};
	}

	#if !defined(NDEBUG)
	std::stringstream ss {};
	ss << "Chunk {";
	ss << pos;
	ss << "}";
	std::string name = ss.str();

	buffer.setDebugName(system.device, name.c_str());
	#endif
}

void WorldRenderer::ChunkBuffer::draw(QueryPool& pool, CommandRecorder& recorder) {
	recorder.beginQuery(pool, identifier);
	buffer.draw(recorder);
	recorder.endQuery(pool, identifier);
}

void WorldRenderer::ChunkBuffer::dispose(RenderSystem& system) {
	system.defer([this, &system] () {
		system.predicate_allocator.free(identifier);
		this->buffer.close();
		delete this;
	});
}

glm::vec3 WorldRenderer::ChunkBuffer::getOffset() const {
	return glm::vec3 {pos * Chunk::size} - 0.5f;
}

bool WorldRenderer::ChunkBuffer::getOcclusion(QueryPool& pool) const {
	return pool.read(this->identifier).get(0);
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

void WorldRenderer::draw(CommandRecorder& recorder, Frustum& frustum, Camera& camera) {

	world_frustum_count = 0;
	Frame& frame = system.getFrame();
	VkExtent2D extent = system.swapchain.vk_extent; // clean up ?
	glm::vec3 origin = camera.getPosition() / (float) Chunk::size;

	world_occlusion_count = system.predicate_allocator.remaining();

	relative = buffers.values();
	std::sort(relative.begin(), relative.end(), [origin] (const auto& lhs, const auto& rhs) {
		return glm::distance2(glm::vec3 {lhs.first}, origin) < glm::distance2(glm::vec3 {rhs.first}, origin);
	});

	std::vector<ChunkBuffer*> discarded;

	// begin Terrain Render Pass
	recorder.beginRenderPass(system.terrain_pass, system.terrain_framebuffer, extent);
	recorder.bindPipeline(system.pipeline_terrain);
	recorder.bindDescriptorSet(frame.set_0);
	recorder.insertDebugLabel("Draw Visible");

	for (auto& [pos, chunk] : relative) {

		glm::vec3 offset = chunk->getOffset();
		bool visible = chunk->getOcclusion(frame.occlusion_query);

		if (visible) {
			chunk->draw(frame.occlusion_query, recorder);
			world_frustum_count ++;
			continue;
		}

		if (frustum.testBox3D(offset, offset + (float) Chunk::size)) {
			if (chunk->count < 100) {
				chunk->draw(frame.occlusion_query, recorder);
				world_frustum_count ++;
				continue;
			}

			discarded.push_back(chunk);
		}

	}

	// Here we SHOULD wait for terrain upload to complete but we do
	// that earlier (before we begin world rendering) that should preferably not be so

	recorder.insertDebugLabel("Draw Awaiting");

	// render all new chunks and copy them into static chunk map
	for (ChunkBuffer* chunk : awaiting.read()) {
		buffers[chunk->pos] = chunk;
		glm::vec3 offset = chunk->getOffset();

		if (frustum.testBox3D(offset, offset + (float) Chunk::size)) {
			chunk->draw(frame.occlusion_query, recorder);
			world_frustum_count ++;
		}
	}

	// now we will test the chunks deemed occluded in the previous cycle
	recorder.nextSubpass();
	recorder.bindPipeline(system.pipeline_occlude);
	recorder.bindBuffer(system.chunk_box);
	recorder.insertDebugLabel("Draw Bounding");

	// we need to rebind the same descriptor set here as the pipeline layout is different
	recorder.bindDescriptorSet(frame.set_0);

	for (int i = 0; i < (int) discarded.size(); i ++) {
		ChunkBuffer* chunk = discarded[i];
		glm::vec3 offset = chunk->getOffset();

		recorder.beginQuery(system.predicate_query, i);
		recorder.writePushConstant(system.push_constant_occlude, glm::value_ptr(offset));
		recorder.draw(36);
		recorder.endQuery(system.predicate_query, i);
	}

	recorder.endRenderPass();
	recorder.copyQueryToBuffer(system.chunk_predicates, system.predicate_query, 0, discarded.size());
	// TODO wait for copy to complete

	recorder.beginRenderPass(system.conditional_pass, system.conditional_framebuffer, extent);
	recorder.bindPipeline(system.pipeline_conditional);
	recorder.bindDescriptorSet(frame.set_0);
	recorder.insertDebugLabel("Draw Conditional");

	for (int i = 0; i < (int) discarded.size(); i ++) {
		ChunkBuffer* chunk = discarded[i];
		recorder.beginConditional(system.chunk_predicates, i * sizeof(uint32_t));
		chunk->draw(frame.occlusion_query, recorder);
		recorder.endConditional();
	}

	recorder.endRenderPass();

}

void WorldRenderer::submitChunk(glm::ivec3 pos, std::vector<VertexTerrain>& mesh) {
	auto* chunk = new ChunkBuffer(system, pos, mesh);
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