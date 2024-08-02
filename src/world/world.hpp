#pragma once

#include "external.hpp"
#include "chunk.hpp"
#include "util/logger.hpp"
#include "util/direction.hpp"
#include "util/bits.hpp"

struct AccessError {

	const int x, y, z;

	AccessError(int x, int y, int z)
	: x(x), y(y), z(z) {}

};

class WorldGenerator;

class World {

	private:

		std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;
		std::unordered_map<glm::ivec3, uint8_t> updates;

		/// Checks if the chunk `chunk` can be submitted for rendering
		bool isChunkRenderReady(glm::ivec3 chunk) const;

	public:

		/// Used by the WorldRenderer, iterates and clears the chunk update set
		template <typename Func>
		void consumeUpdates(Func func) {

			std::unordered_set<glm::ivec3> set;
			set.reserve(updates.size() * 2);

			// propagate updates
			for (auto& [pos, directions] : updates) {
				for (uint8_t direction : Bits::decompose(directions)) {
					glm::ivec3 neighbour = Direction::offset(direction) + pos;

					if (isChunkRenderReady(neighbour)) {
						set.insert(neighbour);
					}
				}

				if (isChunkRenderReady(pos)) {
					set.insert(pos);
				}
			}

			updates.clear();

			// call once for each updated chunk
			for (auto pos : set) {
				func(pos);
			}
		}

		/// Notifies the world that the content of the `chunk` changed
		/// and which neighbours are also affected and also needs to be remeshed
		void pushChunkUpdate(glm::ivec3 chunk, uint8_t directions);

		/// Update the world
		/// manages chunk loading and unloading
		void update(WorldGenerator& generator, glm::ivec3 origin, float radius);

		/// Returns the chunk at the specified chunk coordinates
		/// if the chunk is not loaded returns an empty weak_ptr
		std::weak_ptr<Chunk> getChunk(int cx, int cy, int cz);

		/// returns the block at the specified world coordinates,
		/// if the containing chunk is not loaded throws AccessError
		uint32_t getBlock(int x, int y, int z);

		/// returns the block at the specified world coordinates,
		/// if the containing chunk is not loaded throws AccessError
		void setBlock(int x, int y, int z, uint32_t block);

};
