
#include "pool.hpp"
#include "util/logger.hpp"
#include "util/direction.hpp"
#include "client/renderer.hpp"
#include "world/world.hpp"
#include "renderer.hpp"
#include "mesher.hpp"
#include "view.hpp"

/*
 * ChunkRenderPool::UpdateRequest
 */

ChunkRenderPool::UpdateRequest::UpdateRequest(glm::ivec3 pos)
:pos(pos), timer() {}

glm::ivec3 ChunkRenderPool::UpdateRequest::unpack() const {
	logger::info("Remesh task waited: ", timer.milliseconds(), "ms");
	return pos;
}

/*
 * ChunkRenderPool
 */

std::pair<bool, glm::ivec3> ChunkRenderPool::pop() {
	if (!high_queue.empty()) {
		glm::ivec3 chunk = high_queue.front().unpack();
		high_queue.pop();
		set.erase(chunk);
		return {true, chunk};
	}

	if (!low_queue.empty()) {
		glm::ivec3 chunk = low_queue.front().unpack();
		low_queue.pop();
		set.erase(chunk);
		return {true, chunk};
	}

	return {false, {}};
}

bool ChunkRenderPool::empty() {
	return set.empty();
}

void ChunkRenderPool::emitChunk(std::vector<VertexTerrain>& mesh, ChunkFaceBuffer& buffer, std::shared_ptr<Chunk> chunk) {
	mesh.clear();

	logger::info("Chunk meshing took: ", Timer::of([&] {

		WorldRenderView view {world, chunk, Direction::ALL};
		const SpriteArray& array = system.assets.state->array;

		// failed to lock the view, this chunk must have fallen outside the render distance
		if (view.failed()) {
			return;
		}

		// pass control to the greedy meshing algorithm
		GreedyMesher::emitChunk(mesh, buffer, chunk, view, array);

	}).milliseconds(), "ms");

	renderer.submitChunk(chunk->pos, mesh);
}

void ChunkRenderPool::run() {

	bool got = false;
	glm::ivec3 pos;
	std::vector<VertexTerrain> mesh;
	mesh.reserve(4096);
	ChunkFaceBuffer buffer;

	while (true) {
		{
			std::unique_lock<std::mutex> lock(mutex);

			// wait for task to appear in tasks queue or for the stop sequence to begin
			condition.wait(lock, [this] { return stop || !empty(); });

			if (stop && empty()) {
				return;
			}

			std::tie(got, pos) = pop();
		}

		if (!got) {
			continue;
		}

		if (const auto chunk = world.getChunk(pos.x, pos.y, pos.z).lock()) {
			if (!chunk->empty()) {
				emitChunk(mesh, buffer, chunk);
			}
		}
	}
}

ChunkRenderPool::ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world)
: renderer(renderer), system(system), world(world) {
	for (int i = 0; i < (int) TaskPool::optimal(); i ++) {
		workers.emplace_back(&ChunkRenderPool::run, this);
	}
}

void ChunkRenderPool::push(glm::ivec3 chunk, bool important) {
	{
		std::lock_guard lock {mutex};

		if (set.contains(chunk)) {
			logger::info("Chunk update request rejected!");
			return;
		}

		(important ? high_queue : low_queue).emplace(chunk);
		set.insert(chunk);
	}

	// don't wait while calling notify,
	// it's not invalid but MAY be non-optimal
	condition.notify_one();
}

void ChunkRenderPool::close() {
	logger::info("Stopping chunk meshing worker pool...");
	{
		std::lock_guard lock {mutex};
		stop = true;
	}

	// don't wait while calling notify,
	// it's not invalid but MAY be non-optimal
	condition.notify_all();

	for (std::thread& thread : workers) {
		thread.join();
	}

	workers.clear();
}