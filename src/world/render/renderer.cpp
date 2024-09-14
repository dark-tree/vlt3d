
#include "renderer.hpp"
#include "mesher.hpp"
#include "world/view.hpp"

// TODO TODO TODO TODO
std::atomic_int world_vertex_count = 0;
std::atomic_int world_chunk_count = 0;
std::atomic_int world_visible_count = 0;
std::atomic_int world_occlusion_count = 0;

/*
 * ChunkBuffer
 */

WorldRenderer::ChunkBuffer::ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const MeshEmitterSet& emitters, uint64_t stamp)
: stamp(stamp), pos(pos), identifier(system.predicate_allocator.allocate()), buffer(system, emitters.bytes()) {
	emitters.writeToBuffer(buffer, region_begin, region_count);

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

void WorldRenderer::ChunkBuffer::draw(QueryPool& pool, CommandRecorder& recorder, glm::vec3 cam, bool cull) {

	recorder.beginQuery(pool, identifier);

	if (cull) {
		bool mask[7];

		glm::vec3 pos = getOffset();
		glm::vec3 end = pos + 32.0f;

		mask[0] = cam.x < end.x; // west
		mask[1] = cam.x > pos.x; // east
		mask[2] = cam.y < end.y; // down
		mask[3] = cam.y > pos.y; // up
		mask[4] = cam.z < end.z; // north
		mask[5] = cam.z > pos.z; // south
		mask[6] = true; // unaligned

		recorder.beginQuery(pool, identifier);
		for (int i = 0; i < 7; i ++) {
			if (mask[i]) {
				const uint32_t start = region_begin[i];
				const uint32_t count = region_count[i];

				if (count) {
					recorder.bindVertexBuffer(buffer.getBuffer(), start * sizeof(VertexTerrain));
					recorder.draw(count);
				}
			}
		}
	} else {
		// it looks like doing ~3x the drawcalls is still faster
		// if we reduce the amount of geometry in total.
		buffer.draw(recorder);
	}

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

long WorldRenderer::ChunkBuffer::getCount() const {
	return buffer.getCount();
}

bool WorldRenderer::ChunkBuffer::shouldReplace(ChunkBuffer* chunk) const {
	return stamp > chunk->stamp;
}

/*
 * WorldRenderer
 */

void WorldRenderer::replaceChunk(glm::ivec3 pos, NULLABLE ChunkBuffer* chunk) {
	auto it = buffers.find(pos);

	if (it != buffers.end()) {
		ChunkBuffer* buffer = buffers.extract(it).second;
		world_vertex_count -= buffer->getCount();
		buffer->dispose(system);
	}

	if (chunk) {
		buffers.emplace(pos, chunk);
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
			replaceChunk(pos, nullptr);
		}

		erasures.clear();
	}

	// iterate all chunks that were updated this frame and need to be re-meshed
	world.consumeUpdates([&] (WorldView&& view, bool important) {
		mesher.push(std::move(view), important, unique_stamp ++);
	});

	// first upload all awaiting meshes so that the PCI has something to do
	for (auto [pos, chunk] : awaiting.read()) {
		chunk->buffer.upload(recorder);
	}

}

void WorldRenderer::draw(CommandRecorder& recorder, Frustum& frustum, Camera& camera) {

	Frame& frame = system.getFrame();
	VkExtent2D extent = system.swapchain.vk_extent; // clean up ?
	glm::vec3 camera_pos = camera.getPosition();
	glm::vec3 origin = camera_pos / (float) Chunk::size;

	world_occlusion_count = system.predicate_allocator.remaining();
	world_chunk_count = buffers.size();

	relative.clear();
	std::vector<ChunkBuffer*> conditional;

	// begin Terrain Render Pass
	recorder.beginRenderPass(system.terrain_pass, system.terrain_framebuffer, extent);
	recorder.bindPipeline(system.pipeline_terrain);
	recorder.bindDescriptorSet(frame.set_0);
	recorder.insertDebugLabel("Draw Visible");

	// divide the chunks into 'visible', 'discarded', and 'conditional'
	for (auto& [pos, chunk] : buffers.values()) {

		glm::vec3 offset = chunk->getOffset();
		bool visible = chunk->getOcclusion(frame.occlusion_query);
		float distance = glm::distance2(glm::vec3 {pos} + 0.5f, origin);

		if (visible) {
			relative.emplace_back(distance, chunk);
			continue;
		}

		if (frustum.testBox3D(offset, offset + (float) Chunk::size)) {
			if (chunk->getCount() < 100) {
				relative.emplace_back(distance, chunk);
				continue;
			}

			conditional.push_back(chunk);
		}

		// if we got here the chunk is discarded

	}

	world_visible_count = relative.size() + awaiting.read().size();

	// sort by distance (closest to furthest)
	std::sort(relative.begin(), relative.end(), [] (const auto& lhs, const auto& rhs) {
		return lhs.first < rhs.first;
	});

	for (auto& [pos, chunk] : relative) {
		chunk->draw(frame.occlusion_query, recorder, camera_pos, true);
	}

	// Here we SHOULD wait for terrain upload to complete but we do
	// that earlier (before we begin world rendering) that should preferably not be so

	recorder.insertDebugLabel("Draw Awaiting");

	// render all new chunks and copy them into static chunk map
	for (auto [pos, chunk] : awaiting.read()) {
		replaceChunk(pos, chunk);
		glm::vec3 offset = chunk->getOffset();

		if (frustum.testBox3D(offset, offset + (float) Chunk::size)) {
			chunk->draw(frame.occlusion_query, recorder, camera_pos, false);
		}
	}

	// now we will test the chunks deemed occluded in the previous cycle
	recorder.nextSubpass();
	recorder.bindPipeline(system.pipeline_occlude);
	recorder.bindVertexBuffer(system.chunk_box);
	recorder.insertDebugLabel("Draw Bounding");

	// we need to rebind the same descriptor set here as the pipeline layout is different
	recorder.bindDescriptorSet(frame.set_0);

	for (int i = 0; i < (int) conditional.size(); i ++) {
		ChunkBuffer* chunk = conditional[i];
		glm::vec3 offset = chunk->getOffset();

		recorder.beginQuery(system.predicate_query, i);
		recorder.writePushConstant(system.push_constant_occlude, glm::value_ptr(offset));
		recorder.draw(36);
		recorder.endQuery(system.predicate_query, i);
	}

	recorder.endRenderPass();
	recorder.copyQueryToBuffer(system.chunk_predicates, system.predicate_query, 0, conditional.size());
	recorder.bufferTransferBarrier(VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT);

	recorder.beginRenderPass(system.conditional_pass, system.conditional_framebuffer, extent);
	recorder.bindPipeline(system.pipeline_conditional);
	recorder.bindDescriptorSet(frame.set_0);
	recorder.insertDebugLabel("Draw Conditional");

	for (int i = 0; i < (int) conditional.size(); i ++) {
		ChunkBuffer* chunk = conditional[i];
		recorder.beginConditional(system.chunk_predicates, i * sizeof(uint32_t));
		chunk->draw(frame.occlusion_query, recorder, camera_pos, false);
		recorder.endConditional();
	}

	recorder.endRenderPass();

}

void WorldRenderer::submitChunk(glm::ivec3 pos, const MeshEmitterSet& emitters, uint64_t stamp) {
	auto* chunk = new ChunkBuffer(system, pos, emitters, stamp);
	world_vertex_count += chunk->getCount();

	std::lock_guard lock {submit_mutex};
	allocations.push_back(chunk->buffer.getCount());

	auto& map = awaiting.write();
	auto it = map.find(pos);

	if (it != map.end()) {
		auto* older_chunk = (*it).second;

		if (chunk->shouldReplace(older_chunk)) {
			world_vertex_count -= older_chunk->getCount();
			older_chunk->dispose(system);
			map.erase(it);
			// fallthrough
		} else {
			world_vertex_count -= chunk->getCount();
			chunk->dispose(system);
			return;
		}
	}

	map.emplace(pos, chunk);
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

	for (auto [pos, chunk] : awaiting.read()) {
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