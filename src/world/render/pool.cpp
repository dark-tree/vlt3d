
#include "pool.hpp"
#include "util/logger.hpp"
#include "util/type/direction.hpp"
#include "client/renderer.hpp"
#include "world/world.hpp"
#include "renderer.hpp"
#include "mesher.hpp"
#include "world/view.hpp"

/*
 * ChunkRenderPool::UpdateRequest
 */

ChunkRenderPool::UpdateRequest::UpdateRequest(WorldView&& view, uint64_t stamp)
: view(view), timer(), stamp(stamp) {}

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

uint64_t ChunkRenderPool::UpdateRequest::getStamp() const {
	return stamp;
}

/*
 * ChunkRenderPool
 */

std::pair<bool, ChunkRenderPool::UpdateRequest> ChunkRenderPool::pop() {
	if (!high_queue.empty()) {
		UpdateRequest request = high_queue.front();
		high_queue.pop();
		set.erase(request.origin());
		return {true, std::move(request)};
	}

	if (!low_queue.empty()) {
		UpdateRequest request = low_queue.front();
		low_queue.pop();
		set.erase(request.origin());
		return {true, std::move(request)};
	}

	return {false, {}};
}

bool ChunkRenderPool::empty() {
	return set.empty();
}

void ChunkRenderPool::emitChunk(MeshEmitterSet& mesh, ChunkFaceBuffer& buffer, WorldView& view, uint64_t stamp) {
	mesh.clear();
	GreedyMesher::emitChunk(mesh, buffer, view, system.assets.state->array);

	if (!mesh.empty()) {
		renderer.submitChunk(view.origin(), mesh, stamp);
	}
}

void ChunkRenderPool::run() {

	bool got = false;
	UpdateRequest request;
	MeshEmitterSet emitters {1024};
	ChunkFaceBuffer buffer;

	while (true) {
		{
			std::unique_lock<std::mutex> lock(mutex);

			// wait for task to appear in tasks queue or for the stop sequence to begin
			condition.wait(lock, [this] { return stop || !empty(); });

			if (stop && empty()) {
				return;
			}

			std::tie(got, request) = pop();
		}

		if (!got) {
			continue;
		}

		WorldView view = request.unpack();

		if (!view.getOriginChunk()->empty()) {
			emitChunk(emitters, buffer, view, request.getStamp());
		}
	}
}

ChunkRenderPool::ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world)
: renderer(renderer), system(system), world(world) {
	for (int i = 0; i < (int) TaskPool::optimal(); i ++) {
		workers.emplace_back(&ChunkRenderPool::run, this);
	}
}

void ChunkRenderPool::push(WorldView&& view, bool important, uint64_t stamp) {
	{
		std::lock_guard lock {mutex};
		glm::ivec3 chunk = view.origin();

		if (set.contains(chunk)) {
			logger::info("Chunk update request rejected!");
			return;
		}

		(important ? high_queue : low_queue).emplace(std::move(view), stamp);
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