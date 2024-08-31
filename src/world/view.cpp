
#include "view.hpp"
#include "util/bits.hpp"
#include "world/chunk.hpp"
#include "world/world.hpp"

/*
 * WorldRenderView
 */

int WorldView::indexOf(int x, int y, int z) const {
	int dx = (center_chunk.x - x) + 1;
	int dy = (center_chunk.y - y) + 1;
	int dz = (center_chunk.z - z) + 1;

	return dx + dz * 3 + dy * 9;
}

WorldView::WorldView(World& world, const std::shared_ptr<Chunk>& center, Direction directions) {
	if (!center) {
		failed_to_lock = true;
		return;
	}

	this->center_chunk = center->pos;
	chunks[indexOf(center_chunk.x, center_chunk.y, center_chunk.z)] = center;

	for (Direction direction : Bits::decompose<Direction::field_type>(directions)) {
		glm::ivec3 key = center_chunk + Direction::offset(direction);
		std::shared_ptr<Chunk> lock = world.getUnsafeChunk(key.x, key.y, key.z).lock();

		// terrain got unloaded, we are no longer in the view distance
		if (!lock) {
			failed_to_lock = true;
			return;
		}

		chunks[indexOf(key.x, key.y, key.z)] = std::move(lock);
	}

}

bool WorldView::failed() const {
	return failed_to_lock;
}

glm::ivec3 WorldView::origin() const {
	return center_chunk;
}

Block WorldView::getBlock(int x, int y, int z) {
	int cx = x >> Chunk::bits;
	int cy = y >> Chunk::bits;
	int cz = z >> Chunk::bits;

	return getChunk(cx, cy, cz)->getBlock(x & Chunk::mask, y & Chunk::mask, z & Chunk::mask);
}

Chunk* WorldView::getChunk(int cx, int cy, int cz) {
	return chunks[indexOf(cx, cy, cz)].get();
}

Chunk* WorldView::getOriginChunk() {
	return getChunk(origin().x, origin().y, origin().z);
}