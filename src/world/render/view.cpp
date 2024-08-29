
#include "view.hpp"
#include "util/bits.hpp"
#include "world/chunk.hpp"
#include "world/world.hpp"

/*
 * WorldRenderView
 */

WorldRenderView::WorldRenderView(World& world, const std::shared_ptr<Chunk>& center, Direction directions) {
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