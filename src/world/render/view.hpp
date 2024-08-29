#pragma once

#include "external.hpp"
#include "util/direction.hpp"

class World;
class Chunk;
class Block;

class WorldRenderView {

	private:

		int indexOf(int x, int y, int z) {
			int dx = (origin.x - x) + 1;
			int dy = (origin.y - y) + 1;
			int dz = (origin.z - z) + 1;

			return dx + dz * 3 + dy * 9;
		}

		glm::ivec3 origin;
		bool failed_to_lock = false;
		std::shared_ptr<Chunk> chunks[3*3*3];

	public:

		WorldRenderView(World& world, const std::shared_ptr<Chunk>& center, Direction directions);

		bool failed() const;
		Block getBlock(int x, int y, int z);

};