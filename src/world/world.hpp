#pragma once

#include "external.hpp"
#include "chunk.hpp"
#include "util/logger.hpp"
#include "util/type/direction.hpp"
#include "raycast.hpp"
#include "util/collection/ring.hpp"
#include "view.hpp"
#include "column.hpp"

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
			static_assert(sizeof(uint8_t) >= sizeof(Direction::mask_type), "The ChunkUpdate and Direction flags need to be able to be combined");

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

		RingBuffer<double, 256> times;

		std::mutex chunks_mutex;
		ankerl::unordered_dense::map<glm::ivec2, ChunkColumn> columns;

		std::mutex updates_mutex;
		ankerl::unordered_dense::map<glm::ivec3, uint8_t> updates;

		/// Simple utility to iterate a plane with ever expanding concentric square rings
		template <typename Func>
		void planeRingIterator(int ring, Func func) {
			if (ring == 0) {
				func(0, 0);
				return;
			}

			for (int i = -ring; i <= ring; i ++) {
				func(i, -ring);
				func(i, +ring);
				func(-ring, i);
				func(+ring, i);
			}
		}

		/// Simple utility to iterate a line with ever expanding concentric square rings
		template <typename Func>
		void lineRingIterator(int radius, Func func) {
			func(0);

			for (int i = 1; i <= radius; i ++) {
				func(-i);
				func(+i);
			}
		}

	public:

		/// Used by the WorldRenderer, iterates and clears the chunk update set
		template <typename Func>
		void consumeUpdates(Func func) {
			std::unordered_set<ChunkUpdate, ChunkUpdate::Hasher> set;
			set.reserve(updates.size() * 2);

			{
				std::lock_guard lock {updates_mutex};

				// propagate updates
				for (auto& [pos, flags] : updates) {
					for (Direction direction : Direction::decompose(flags & Direction::ALL)) {
						set.emplace(Direction::offset(direction) + pos, flags & ChunkUpdate::IMPORTANT);
					}

					set.emplace(pos, flags & ChunkUpdate::IMPORTANT);
				}

				updates.clear();
			}

			if (!set.empty()) {
				logger::debug("Requesting remeshing of ", set.size(), " chunks");
			}

			// call once for each updated chunk
			std::lock_guard lock {chunks_mutex};

			for (auto update : set) {
				WorldView view = getUnsafeView(update.pos, Direction::ALL);

				if (!view.failed()) {
					func(std::move(view), update.important);
				}
			}
		}

		/// Notifies the world that the content of the `chunk` changed
		/// and which neighbours are also affected and also needs to be remeshed
		void pushChunkUpdate(glm::ivec3 chunk, uint8_t flags);

		/// Update the world
		/// manages chunk loading and unloading
		void update(WorldGenerator& generator, glm::ivec3 origin, float radius, float vertical);

		/// Creates a new WorldView around the specified chunk
		/// this functions is unsafe - expects the caller to lock chunks_mutex
		WorldView getUnsafeView(glm::ivec3 chunk, Direction directions);

		/// Creates a new WorldView, right now this expects external the chunks_mutex to be locked by the caller
		std::weak_ptr<Chunk> getUnsafeChunk(int cx, int cy, int cz);

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
