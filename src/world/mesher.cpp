
#include "mesher.hpp"
#include "renderer.hpp"

/*
 * WorldRenderView
 */

WorldRenderView::WorldRenderView(World& world, std::shared_ptr<Chunk>& center, Direction directions) {
	if (!center) {
		failed_to_lock = true;
		return;
	}

	this->origin = center->pos;
	chunks[indexOf(origin.x, origin.y, origin.z)] = center;

	for (Direction direction : Bits::decompose<Direction::field_type>(directions)) {
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

template <Normal normal>
void emitQuad(std::vector<VertexTerrain>& mesh, float x, float y, float z, float slice, float alpha, float beta, float width, float height, uint16_t index, BakedSprite sprite) {

	const float aps = -(alpha - 0.5f);
	const float bps = -(beta - 0.5f);
	const float apo = width - aps;
	const float bpo = height - bps;

	BakedSprite scaled {sprite.u1, sprite.v1, sprite.u2 * width, sprite.v2 * height};

	if constexpr (normal == Normal::EAST) {
		x += slice;

		mesh.emplace_back(x + 0.5, y - aps, z - bps, scaled.u1, scaled.v2, index, 255, 0, 0, Normal::EAST);
		mesh.emplace_back(x + 0.5, y + apo, z + bpo, scaled.u2, scaled.v1, index, 0, 255, 0, Normal::EAST);
		mesh.emplace_back(x + 0.5, y - aps, z + bpo, scaled.u2, scaled.v2, index, 0, 0, 255, Normal::EAST);

		mesh.emplace_back(x + 0.5, y - aps, z - bps, scaled.u1, scaled.v2, index, 255, 0, 0, Normal::EAST);
		mesh.emplace_back(x + 0.5, y + apo, z - bps, scaled.u1, scaled.v1, index, 0, 255, 0, Normal::EAST);
		mesh.emplace_back(x + 0.5, y + apo, z + bpo, scaled.u2, scaled.v1, index, 0, 0, 255, Normal::EAST);
	}

	if constexpr (normal == Normal::WEST) {
		x += slice;

		mesh.emplace_back(x - 0.5, y - aps, z - bps, scaled.u1, scaled.v2, index, 255, 0, 0, Normal::WEST);
		mesh.emplace_back(x - 0.5, y - aps, z + bpo, scaled.u2, scaled.v2, index, 0, 255, 0, Normal::WEST);
		mesh.emplace_back(x - 0.5, y + apo, z + bpo, scaled.u2, scaled.v1, index, 0, 0, 255, Normal::WEST);

		mesh.emplace_back(x - 0.5, y - aps, z - bps, scaled.u1, scaled.v2, index, 255, 0, 0, Normal::WEST);
		mesh.emplace_back(x - 0.5, y + apo, z + bpo, scaled.u2, scaled.v1, index, 0, 255, 0, Normal::WEST);
		mesh.emplace_back(x - 0.5, y + apo, z - bps, scaled.u1, scaled.v1, index, 0, 0, 255, Normal::WEST);
	}

	if constexpr (normal == Normal::UP) {
		y += slice;

		mesh.emplace_back(x - aps, y + 0.5, z - bps, scaled.u1, scaled.v1, index, 255, 0, 0, Normal::UP);
		mesh.emplace_back(x - aps, y + 0.5, z + bpo, scaled.u1, scaled.v2, index, 0, 255, 0, Normal::UP);
		mesh.emplace_back(x + apo, y + 0.5, z + bpo, scaled.u2, scaled.v2, index, 0, 0, 255, Normal::UP);

		mesh.emplace_back(x - aps, y + 0.5, z - bps, scaled.u1, scaled.v1, index, 255, 0, 0, Normal::UP);
		mesh.emplace_back(x + apo, y + 0.5, z + bpo, scaled.u2, scaled.v2, index, 0, 255, 0, Normal::UP);
		mesh.emplace_back(x + apo, y + 0.5, z - bps, scaled.u2, scaled.v1, index, 0, 0, 255, Normal::UP);
	}

	if constexpr (normal == Normal::DOWN) {
		y += slice;

		mesh.emplace_back(x - aps, y - 0.5, z - bps, scaled.u1, scaled.v1, index, 255, 0, 0, Normal::DOWN);
		mesh.emplace_back(x + apo, y - 0.5, z + bpo, scaled.u2, scaled.v2, index, 0, 255, 0, Normal::DOWN);
		mesh.emplace_back(x - aps, y - 0.5, z + bpo, scaled.u1, scaled.v2, index, 0, 0, 255, Normal::DOWN);

		mesh.emplace_back(x - aps, y - 0.5, z - bps, scaled.u1, scaled.v1, index, 255, 0, 0, Normal::DOWN);
		mesh.emplace_back(x + apo, y - 0.5, z - bps, scaled.u2, scaled.v1, index, 0, 255, 0, Normal::DOWN);
		mesh.emplace_back(x + apo, y - 0.5, z + bpo, scaled.u2, scaled.v2, index, 0, 0, 255, Normal::DOWN);
	}

	if constexpr (normal == Normal::SOUTH) {
		z += slice;

		mesh.emplace_back(x - aps, y - bps, z + 0.5, scaled.u1, scaled.v2, index, 255, 0, 0, normal);
		mesh.emplace_back(x + apo, y + bpo, z + 0.5, scaled.u2, scaled.v1, index, 0, 255, 0, normal);
		mesh.emplace_back(x - aps, y + bpo, z + 0.5, scaled.u1, scaled.v1, index, 0, 0, 255, normal);

		mesh.emplace_back(x - aps, y - bps, z + 0.5, scaled.u1, scaled.v2, index, 255, 0, 0, normal);
		mesh.emplace_back(x + apo, y - bps, z + 0.5, scaled.u2, scaled.v2, index, 0, 255, 0, normal);
		mesh.emplace_back(x + apo, y + bpo, z + 0.5, scaled.u2, scaled.v1, index, 0, 0, 255, normal);
	}

	if constexpr (normal == Normal::NORTH) {
		z += slice;

		mesh.emplace_back(x - aps, y - bps, z - 0.5, scaled.u1, scaled.v2, index, 255, 0, 0, Normal::NORTH);
		mesh.emplace_back(x - aps, y + bpo, z - 0.5, scaled.u1, scaled.v1, index, 0, 255, 0, Normal::NORTH);
		mesh.emplace_back(x + apo, y + bpo, z - 0.5, scaled.u2, scaled.v1, index, 0, 0, 255, Normal::NORTH);

		mesh.emplace_back(x - aps, y - bps, z - 0.5, scaled.u1, scaled.v2, index, 255, 0, 0, Normal::NORTH);
		mesh.emplace_back(x + apo, y + bpo, z - 0.5, scaled.u2, scaled.v1, index, 0, 255, 0, Normal::NORTH);
		mesh.emplace_back(x + apo, y - bps, z - 0.5, scaled.u2, scaled.v2, index, 0, 0, 255, Normal::NORTH);
	}

}

struct QuadDelegate {
	uint16_t offset;
	uint16_t index;
	uint16_t streak;
	uint16_t extend;

	bool canMergeWith(const QuadDelegate& other) const {
		return (other.index == index) && (other.streak == streak) && (other.offset == offset);
	}
};

template <Normal normal>
void emitPlane(std::vector<VertexTerrain>& mesh, glm::ivec3 chunk, int slice, ChunkPlane& plane) {
	const BakedSprite sprite = BakedSprite::identity();

	std::vector<QuadDelegate> delegates;
	delegates.emplace_back(0, 0, 1, 1);

	uint32_t back[Chunk::size];
	uint32_t front[Chunk::size];

	for (int a = 0; a < Chunk::size; a ++) {
		size_t prev = 0;

		for (int b = 0; b < Chunk::size; b ++) {
			uint16_t index = plane.at(a, b);

			if (index) {
				QuadDelegate& quad = delegates[prev];

				if (index == quad.index) {
					quad.streak ++;
					front[b] = delegates.size() - 1;
					continue;
				}

				delegates.emplace_back(b, index, 1, 1);
				front[b] = prev = delegates.size() - 1;
				continue;
			}

			prev = 0;
			front[b] = 0;
		}

		if (a != 0) {

			// merge and emit rows
			for (int i = 0; i < Chunk::size;) {
				QuadDelegate& quad = delegates[back[i]];

				if (quad.index) {
					QuadDelegate& next = delegates[front[i]];

					if (quad.canMergeWith(next)) {
						quad.extend ++;
						std::swap(quad, next);
					} else {
						emitQuad<normal>(mesh, chunk.x, chunk.y, chunk.z, slice, a - quad.extend, i, quad.extend, quad.streak, quad.index, sprite);
					}
				}

				i += quad.streak;
			}

		}

		std::swap(back, front);
	}

	// emit the trailing row
	for (int i = 0; i < Chunk::size;) {
		QuadDelegate& quad = delegates[back[i]];

		if (quad.index) {
			emitQuad<normal>(mesh, chunk.x, chunk.y, chunk.z, slice, Chunk::size - quad.extend, i, quad.extend, quad.streak, quad.index, sprite);
		}

		i += quad.streak;
	}
}

void ChunkRenderPool::emitChunk(ChunkFaceBuffer& buffer, std::vector<VertexTerrain>& mesh, std::shared_ptr<Chunk> chunk) {

	logger::info("Chunk meshing took: ", Timer::of([&] {

		WorldRenderView view{world, chunk, Direction::ALL};
		const SpriteArray& array = system.assets.state->array;

		// failed to lock the view, this chunk must have fallen outside the render distance
		if (view.failed()) {
			return;
		}

		buffer.clear();

		int gray_sprite = array.getSpriteIndex("gray");
		int clay_sprite = array.getSpriteIndex("clay");
		int moss_sprite = array.getSpriteIndex("moss");
		int side_sprite = array.getSpriteIndex("side");

		glm::ivec3 world_pos = chunk->pos * Chunk::size;

		for (int z = 0; z < Chunk::size; z++) {
			for (int y = 0; y < Chunk::size; y++) {
				for (int x = 0; x < Chunk::size; x++) {
					Block block = chunk->getBlock(x, y, z);

					if (block.isAir()) {
						continue;
					}

					BlockPlaneView faces = buffer.getBlockView(x, y, z);
					glm::ivec3 pos = world_pos + glm::ivec3{x, y, z};
					int sprite = (block.block_type % 2 == 1) ? gray_sprite : clay_sprite;

					bool west = view.getBlock(pos.x - 1, pos.y, pos.z).isAir();
					bool east = view.getBlock(pos.x + 1, pos.y, pos.z).isAir();
					bool down = view.getBlock(pos.x, pos.y - 1, pos.z).isAir();
					bool up = view.getBlock(pos.x, pos.y + 1, pos.z).isAir();
					bool north = view.getBlock(pos.x, pos.y, pos.z - 1).isAir();
					bool south = view.getBlock(pos.x, pos.y, pos.z + 1).isAir();

					*faces.west = west * sprite;
					*faces.east = east * sprite;
					*faces.down = down * sprite;
					*faces.up = up * sprite;
					*faces.north = north * sprite;
					*faces.south = south * sprite;
				}
			}
		}

		for (int slice = 0; slice < Chunk::size; slice ++) {
			emitPlane<Normal::WEST>(mesh, world_pos, slice, buffer.getX(slice, 0));
			emitPlane<Normal::EAST>(mesh, world_pos, slice, buffer.getX(slice, 1));

			emitPlane<Normal::DOWN>(mesh, world_pos, slice, buffer.getY(slice, 0));
			emitPlane<Normal::UP>(mesh, world_pos, slice, buffer.getY(slice, 1));

			emitPlane<Normal::NORTH>(mesh, world_pos, slice, buffer.getZ(slice, 0));
			emitPlane<Normal::SOUTH>(mesh, world_pos, slice, buffer.getZ(slice, 1));
		}

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
				mesh.clear();
				emitChunk(buffer, mesh, chunk);
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