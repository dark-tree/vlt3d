
#include "pool.hpp"
#include "util/logger.hpp"
#include "util/direction.hpp"
#include "client/renderer.hpp"
#include "world/world.hpp"
#include "renderer.hpp"
#include "mesher.hpp"
#include "world/view.hpp"

/*
 * ChunkRenderPool::UpdateRequest
 */

ChunkRenderPool::UpdateRequest::UpdateRequest(WorldView&& view)
: view(view), timer() {}

glm::ivec3 ChunkRenderPool::UpdateRequest::origin() const {
	double millis = timer.milliseconds();

	if (millis > 100) {
		logger::info("Is the mesher overloaded? Remesh task waited: ", timer.milliseconds(), "ms");
	}

	return view.origin();
}

WorldView&& ChunkRenderPool::UpdateRequest::unpack() {
	return std::move(view);
}

/*
 * ChunkRenderPool
 */

std::pair<bool, WorldView> ChunkRenderPool::pop() {
	if (!high_queue.empty()) {
		UpdateRequest request = high_queue.front();
		high_queue.pop();
		set.erase(request.origin());
		return {true, std::move(request.unpack())};
	}

	if (!low_queue.empty()) {
		UpdateRequest request = low_queue.front();
		low_queue.pop();
		set.erase(request.origin());
		return {true, std::move(request.unpack())};
	}

	return {false, {}};
}

bool ChunkRenderPool::empty() {
	return set.empty();
}

void ChunkRenderPool::emitChunk(std::vector<VertexTerrain>& mesh, ChunkFaceBuffer& buffer, WorldView& view) {
	mesh.clear();
	GreedyMesher::emitChunk(mesh, buffer, view, system.assets.state->array);
	renderer.submitChunk(view.origin(), mesh);
}

void ChunkRenderPool::run() {

	bool got = false;
	WorldView view;
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

			std::tie(got, view) = pop();
		}

		if (!got) {
			continue;
		}

		if (!view.getOriginChunk()->empty()) {
			emitChunk(mesh, buffer, view);
		}
	}
}

ChunkRenderPool::ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world)
: renderer(renderer), system(system), world(world) {
	for (int i = 0; i < (int) TaskPool::optimal(); i ++) {
		workers.emplace_back(&ChunkRenderPool::run, this);
	}
}

void ChunkRenderPool::push(WorldView&& view, bool important) {
	{
		std::lock_guard lock {mutex};
		glm::ivec3 chunk = view.origin();

		if (set.contains(chunk)) {
			logger::info("Chunk update request rejected!");
			return;
		}

		(important ? high_queue : low_queue).emplace(std::move(view));
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