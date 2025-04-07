#pragma once

#include "external.hpp"
#include "util/type/direction.hpp"

class World;
class Chunk;
class Block;

class WorldView {

	private:

		int indexOf(int x, int y, int z) const;

		glm::ivec3 center_chunk;
		bool failed_to_lock = false;
		std::shared_ptr<Chunk> chunks[3*3*3];

	public:

		WorldView() = default;
		WorldView(World& world, const std::shared_ptr<Chunk>& center, Direction directions);

		/// Check if the view was successfully acquired, if not the view should be discarded
		bool failed() const;

		/// Get the chunk coordinates of the center chunk
		glm::ivec3 origin() const;

		/// Get a block at a particular world-pos from the view
		Block getBlock(int x, int y, int z);

		/// Return an pointer ot the chunk, the lifetime is equal to that of the view
		Chunk* getChunk(int cx, int cy, int cz);

		/// Similar to getChunk but returns the chunk pointer to by origin()
		Chunk* getOriginChunk();

};