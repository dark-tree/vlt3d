
#include "mesher.hpp"
#include "renderer.hpp"

/*
 * WorldRenderView
 */

WorldRenderView::WorldRenderView(World& world, std::shared_ptr<Chunk>& center, uint8_t directions) {
	if (!center) {
		failed_to_lock = true;
		return;
	}

	this->origin = center->pos;
	chunks[indexOf(origin.x, origin.y, origin.z)] = center;

	for (uint8_t direction : Bits::decompose(directions)) {
		glm::ivec3 key = origin + Direction::offset(direction);
		std::shared_ptr<Chunk> lock = world.getChunk(key.x, key.y, key.z).lock();

		// terrain got unloaded, we are no longer in the view distance
		if (!lock) {
			failed_to_lock = true;
			return;
		}

		chunks[indexOf(key.x, key.y, key.z)] = std::move(lock);
	}

}

bool WorldRenderView::failed() const {
	return failed_to_lock;
}

Block WorldRenderView::getBlock(int x, int y, int z) {
	int cx = x >> Chunk::bits;
	int cy = y >> Chunk::bits;
	int cz = z >> Chunk::bits;

	return chunks[indexOf(cx, cy, cz)]->getBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask);
}

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

void ChunkRenderPool::emitCube(std::vector<Vertex3D>& mesh, float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite) {
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

void ChunkRenderPool::emitChunk(std::vector<Vertex3D>& mesh, std::shared_ptr<Chunk> chunk) {
	WorldRenderView view {world, chunk, Direction::ALL};
	const Atlas& atlas = system.assets.getAtlas();

	// failed to lock the view, this chunk must have fallen outside the render distance
	if (view.failed()) {
		return;
	}

	for (int z = 0; z < Chunk::size; z ++) {
		for (int y = 0; y < Chunk::size; y ++) {
			for (int x = 0; x < Chunk::size; x ++) {
				Block block = chunk->getBlock(x, y, z);

				if (!block.isAir()) {
					BakedSprite sprite = (block.block_type % 2 == 1) ? atlas.getBakedSprite("vkblob") : atlas.getBakedSprite("digital");
					float shade = std::clamp((chunk->pos.y * Chunk::size + y) / (Chunk::size * 2.0f) + 0.2f, 0.0f, 1.0f);
					glm::ivec3 pos = chunk->pos * Chunk::size + glm::ivec3 {x, y, z};

					emitCube(
						mesh,
						pos.x, pos.y, pos.z,
						shade, shade, shade, // TODO wtf happened to directions here
						view.getBlock(pos.x, pos.y + 1, pos.z).isAir(),
						view.getBlock(pos.x, pos.y - 1, pos.z).isAir(),
						view.getBlock(pos.x + 1, pos.y, pos.z).isAir(),
						view.getBlock(pos.x - 1, pos.y, pos.z).isAir(),
						view.getBlock(pos.x, pos.y, pos.z + 1).isAir(),
						view.getBlock(pos.x, pos.y, pos.z - 1).isAir(),
						sprite
					);
				}
			}
		}
	}

	renderer.submitChunk(chunk->pos, mesh);
}

void ChunkRenderPool::run() {

	bool got = false;
	glm::ivec3 pos;
	std::vector<Vertex3D> mesh;
	mesh.reserve(4096);

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
				mesh.clear();
				emitChunk(mesh, chunk);
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

ChunkRenderPool::~ChunkRenderPool() {
	logger::info("Stopping chunk meshing worker pool...");
	{
		std::lock_guard lock {mutex};
		stop = true;
	}

	// don't wait while calling notify,
	// it's not invalid but MAY be non-optimal
	condition.notify_all();
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