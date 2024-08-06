#pragma once

#include "external.hpp"
#include "chunk.hpp"
#include "util/logger.hpp"
#include "util/direction.hpp"
#include "util/bits.hpp"
#include "raycast.hpp"

struct AccessError : std::exception {

	const int x, y, z;

	AccessError(int x, int y, int z)
	: x(x), y(y), z(z) {}

	const char * what() const noexcept override;

};

class WorldGenerator;
class Raycast;

class World {

	private:

		struct ChunkUpdate {

			static constexpr uint8_t UNIMPORTANT  = 0b00'000000;
			static constexpr uint8_t IMPORTANT    = 0b10'000000;

			/// the flag set used for newly loaded chunks
			static constexpr uint8_t INITIAL_LOAD = UNIMPORTANT | Direction::ALL;

			static_assert((IMPORTANT & Direction::ALL) == 0, "The ChunkUpdate and Direction flags need to be able to be combined");
			static_assert(sizeof(uint8_t) >= sizeof(Direction::field_type), "The ChunkUpdate and Direction flags need to be able to be combined");

			struct Hasher {
				std::size_t operator()(const ChunkUpdate& update) const {
					return std::hash<glm::ivec3>()(update.pos);
				}
			};

			glm::ivec3 pos;
			bool important;

			bool operator ==(ChunkUpdate other) const {
				return pos == other.pos;
			}

			ChunkUpdate(glm::ivec3 pos, bool important)
			: pos(pos), important(important) {}

		};

		std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks;
		std::unordered_map<glm::ivec3, uint8_t> updates;

		/// Checks if the chunk `chunk` can be submitted for rendering
		bool isChunkRenderReady(glm::ivec3 chunk) const;

	public:

		/// Used by the WorldRenderer, iterates and clears the chunk update set
		template <typename Func>
		void consumeUpdates(Func func) {

			std::unordered_set<ChunkUpdate, ChunkUpdate::Hasher> set;
			set.reserve(updates.size() * 2);

			// propagate updates
			for (auto& [pos, flags] : updates) {
				for (uint8_t direction : Bits::decompose<Direction::field_type>(flags & Direction::ALL)) {
					glm::ivec3 neighbour = Direction::offset(direction) + pos;

					if (isChunkRenderReady(neighbour)) {
						set.emplace(neighbour, flags & ChunkUpdate::IMPORTANT);
					}
				}

				if (isChunkRenderReady(pos)) {
					set.emplace(pos, flags & ChunkUpdate::IMPORTANT);
				}
			}

			updates.clear();

			// call once for each updated chunk
			for (auto update : set) {
				logger::debug("Updating chunk ", update.pos, " important: ", update.important);
				func(update.pos, update.important);
			}
		}

		/// Notifies the world that the content of the `chunk` changed
		/// and which neighbours are also affected and also needs to be remeshed
		void pushChunkUpdate(glm::ivec3 chunk, uint8_t flags);

		/// Update the world
		/// manages chunk loading and unloading
		void update(WorldGenerator& generator, glm::ivec3 origin, float radius);

		/// Returns the chunk at the specified chunk coordinates
		/// if the chunk is not loaded returns an empty weak_ptr
		std::weak_ptr<Chunk> getChunk(int cx, int cy, int cz);

		/// returns the block at the specified world coordinates,
		/// if the containing chunk is not loaded throws AccessError
		Block getBlock(int x, int y, int z);

		/// returns the block at the specified world coordinates,
		/// if the containing chunk is not loaded throws AccessError
		void setBlock(int x, int y, int z, Block block);

		/// Casts a ray from the given position until the
		/// distance limit, a block, or an unloaded chunk is encountered
		Raycast raycast(glm::vec3 from, glm::vec3 direction, float distance);

};
