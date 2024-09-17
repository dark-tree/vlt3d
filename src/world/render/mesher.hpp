#pragma once

#include "external.hpp"
#include "client/vertices.hpp"
#include "world/chunk.hpp"
#include "buffer/sprites.hpp"
#include "emitter.hpp"

class SpriteArray;
class WorldView;

struct BlockFaceView {

	uint16_t* west;
	uint16_t* east;
	uint16_t* down;
	uint16_t* up;
	uint16_t* north;
	uint16_t* south;

};

class ChunkPlane {

	private:

		uint16_t faces[Chunk::size * Chunk::size];

	public:

		uint16_t& at(int alpha, int beta);

};

class ChunkFaceBuffer {

	private:

		static constexpr size_t planes_along_axis = Chunk::size;
		static constexpr size_t planes_per_axis = 2;
		static constexpr size_t size = 3 /* 3D */ * planes_along_axis * planes_per_axis;

		ChunkPlane* buffer = nullptr;

		ChunkPlane& get(int value, int offset);

	public:

		ChunkFaceBuffer();
		~ChunkFaceBuffer();

		void clear(uint16_t empty);

		ChunkPlane& getX(int x, int offset);
		ChunkPlane& getY(int y, int offset);
		ChunkPlane& getZ(int z, int offset);

		BlockFaceView getBlockView(int x, int y, int z);

};

/**
 * This class is a container for all the greedy meshing machinery
 * the general walkthrough of the process look like this:
 *
 * <p>
 * First, in `emitChunk`, the chunk content is iterated block-by-block,
 * each block can write one face sprite into 6 2D chunk slices (planes) held in
 * the `ChunkFaceBuffer` - at this step culling is applied. If a face is culled
 * then a special value `GreedyMesher::culled_tile` is written in place of the sprite index.
 *
 * <p>
 * Then, in `emitChunk`, for each 2D chunk slice (plane) (Chunk::size per direction)
 * the `emitPlane` is invoked. This is the method that actually performs the greedy
 * meshing of the 2D data.
 */
class GreedyMesher {

	private:

		/**
		 * Should the mesher try to merge quads in rows harder?
		 * Allows for connecting quads that are separated by culled tiles
		 * @verbatim
		 *
		 * input:   1 X 2 X X X 2 0 with X = culled tiled
		 * with:    1 0 2 2 2 2 2 0
		 * without: 1 0 2 0 0 0 3 0
		 */
		static constexpr bool greedier_rows = true;

		/**
		 * Should the mesher try to merge rows harder?
		 * Allows for connecting of not aligned quads between rows
		 * @verbatim
		 *
		 * input:   front [3 4 4 4 X X X 0], back [1 X 2 2 2 2 2 0] with 1=3, 2=4
		 * with:    2 quads [{off=0, ext=2, str=1, #3}, {off=1, ext=2, str=6, #4}]
		 * without: 3 quads [{off=0, ext=2, str=1, #3}, {off=1, ext=1, str=3, #4}, {off=2, ext=1, str=5, #2}]
		 */
		static constexpr bool greedier_merge = true;

		/**
		 * Maximum amount of culled tiles between two connected quads
		 * within a row in greedier_rows mode
		 */
		static constexpr int greedier_culling_limit = 32;

	private:

		static constexpr uint16_t empty_tile = 0x0000;
		static constexpr uint16_t culled_tile = 0xFFFF;

		/**
		 * Internal structure used to represent a quad as
		 * it is being generated and merged
		 */
		struct QuadDelegate {
			uint16_t offset; // offset from the start of the row, this is where the quad begins
			uint16_t sprite; // the sprite index this quad uses, only sprites with the same sprites can merge
			uint16_t streak; // the width of the quad, in row tiles
			uint16_t extend; // the height of the quad, in rows
			uint16_t prefix; // number of exclusive culled tiles before the start of this quad
			uint16_t suffix; // number of exclusive culled tiles after the end of this quad

			QuadDelegate(uint16_t offset, uint16_t sprite, uint16_t streak, uint16_t prefix, uint16_t suffix)
			: offset(offset), sprite(sprite), streak(streak), extend(1), prefix(prefix), suffix(suffix) {}
		};

		/**
		 * Invokes the callback method for each separate quad in the given row
		 */
		template <typename Func>
		static void forEachQuad(std::vector<QuadDelegate>& delegates, uint32_t row[], Func func) {
			for (int i = 0; i < Chunk::size;) {
				QuadDelegate& quad = delegates[row[i]];

				if (quad.sprite != empty_tile) {
					// `i` and `quad.offset` are not always the same value
					// as prefix and postfix CAN also be imprinted in the row
					i = quad.offset;
					func(i, quad);
				}

				i += quad.streak;
				i += quad.suffix;
			}
		}

	private:

		/**
		 * Internal method used by `emitPlane`, writes as single quad (two triangles) into the given mesh buffer
		 */
		template <Normal normal>
		static void emitQuad(MeshEmitter& mesh, float cx, float cy, float cz, float slice, float alpha, float beta, float width, float height, uint16_t index, BakedSprite sprite) {

			const float aps = -(alpha - 0.5f);
			const float bps = -(beta - 0.5f);
			const float apo = width - aps;
			const float bpo = height - bps;

			BakedSprite swh {sprite.u1, sprite.v1, sprite.u2 * width, sprite.v2 * height};
			BakedSprite shw {sprite.u1, sprite.v1, sprite.u2 * height, sprite.v2 * width};

			if constexpr (normal == Normal::EAST) {
				cx += slice;

				mesh.nextTriangle();
				mesh.pushVertex(cx + 0.5, cy - aps, cz - bps, shw.u1, shw.v2, index, 255, 0, 0, Normal::EAST);
				mesh.pushVertex(cx + 0.5, cy + apo, cz + bpo, shw.u2, shw.v1, index, 0, 255, 0, Normal::EAST);
				mesh.pushVertex(cx + 0.5, cy - aps, cz + bpo, shw.u2, shw.v2, index, 0, 0, 255, Normal::EAST);

				mesh.nextTriangle();
				mesh.pushIndex(0);
				mesh.pushVertex(cx + 0.5, cy + apo, cz - bps, shw.u1, shw.v1, index, 0, 0, 255, Normal::EAST);
				mesh.pushIndex(1);
			}

			if constexpr (normal == Normal::WEST) {
				cx += slice;

				mesh.nextTriangle();
				mesh.pushVertex(cx - 0.5, cy - aps, cz - bps, shw.u1, shw.v2, index, 255, 0, 0, Normal::WEST);
				mesh.pushVertex(cx - 0.5, cy - aps, cz + bpo, shw.u2, shw.v2, index, 0, 255, 0, Normal::WEST);
				mesh.pushVertex(cx - 0.5, cy + apo, cz + bpo, shw.u2, shw.v1, index, 0, 0, 255, Normal::WEST);

				mesh.nextTriangle();
				mesh.pushIndex(0);
				mesh.pushIndex(2);
				mesh.pushVertex(cx - 0.5, cy + apo, cz - bps, shw.u1, shw.v1, index, 0, 255, 0, Normal::WEST);
			}

			if constexpr (normal == Normal::DOWN) {
				cy += slice;

				mesh.nextTriangle();
				mesh.pushVertex(cx - aps, cy - 0.5, cz - bps, swh.u1, swh.v1, index, 255, 0, 0, Normal::DOWN);
				mesh.pushVertex(cx + apo, cy - 0.5, cz + bpo, swh.u2, swh.v2, index, 0, 255, 0, Normal::DOWN);
				mesh.pushVertex(cx - aps, cy - 0.5, cz + bpo, swh.u1, swh.v2, index, 0, 0, 255, Normal::DOWN);

				mesh.nextTriangle();
				mesh.pushIndex(0);
				mesh.pushVertex(cx + apo, cy - 0.5, cz - bps, swh.u2, swh.v1, index, 0, 0, 255, Normal::DOWN);
				mesh.pushIndex(1);
			}

			if constexpr (normal == Normal::UP) {
				cy += slice;

				mesh.nextTriangle();
				mesh.pushVertex(cx - aps, cy + 0.5, cz - bps, swh.u1, swh.v1, index, 255, 0, 0, Normal::UP);
				mesh.pushVertex(cx - aps, cy + 0.5, cz + bpo, swh.u1, swh.v2, index, 0, 255, 0, Normal::UP);
				mesh.pushVertex(cx + apo, cy + 0.5, cz + bpo, swh.u2, swh.v2, index, 0, 0, 255, Normal::UP);

				mesh.nextTriangle();
				mesh.pushIndex(0);
				mesh.pushIndex(2);
				mesh.pushVertex(cx + apo, cy + 0.5, cz - bps, swh.u2, swh.v1, index, 0, 255, 0, Normal::UP);
			}

			if constexpr (normal == Normal::NORTH) {
				cz += slice;

				mesh.nextTriangle();
				mesh.pushVertex(cx - aps, cy - bps, cz - 0.5, swh.u1, swh.v2, index, 255, 0, 0, Normal::NORTH);
				mesh.pushVertex(cx - aps, cy + bpo, cz - 0.5, swh.u1, swh.v1, index, 0, 255, 0, Normal::NORTH);
				mesh.pushVertex(cx + apo, cy + bpo, cz - 0.5, swh.u2, swh.v1, index, 0, 0, 255, Normal::NORTH);

				mesh.nextTriangle();
				mesh.pushIndex(0);
				mesh.pushIndex(2);
				mesh.pushVertex(cx + apo, cy - bps, cz - 0.5, swh.u2, swh.v2, index, 0, 255, 0, Normal::NORTH);
			}

			if constexpr (normal == Normal::SOUTH) {
				cz += slice;

				mesh.nextTriangle();
				mesh.pushVertex(cx - aps, cy - bps, cz + 0.5, swh.u1, swh.v2, index, 255, 0, 0, normal);
				mesh.pushVertex(cx + apo, cy + bpo, cz + 0.5, swh.u2, swh.v1, index, 0, 255, 0, normal);
				mesh.pushVertex(cx - aps, cy + bpo, cz + 0.5, swh.u1, swh.v1, index, 0, 0, 255, normal);

				mesh.nextTriangle();
				mesh.pushIndex(0);
				mesh.pushVertex(cx + apo, cy - bps, cz + 0.5, swh.u2, swh.v2, index, 0, 0, 255, normal);
				mesh.pushIndex(1);
			}

		}

		/**
		 * Internal method used by `emitChunk` greedily meshes a single 2D face buffer slice
		 */
		template <Normal normal>
		static void emitPlane(MeshEmitter& emitter, glm::ivec3 chunk, int slice, ChunkPlane& plane) {
			const BakedSprite identity = BakedSprite::identity();

			// there is always one empty delegate (with id 0) used to maker air quads
			std::vector<QuadDelegate> delegates;
			delegates.emplace_back(0, empty_tile, 1, 0, 0);

			uint32_t back[Chunk::size];
			uint32_t front[Chunk::size];

			for (int a = 0; a < Chunk::size; a ++) {

				size_t prev = 0;                   // the index of the previous quad delegate that can be merged with
				uint16_t culled = 0;               // counts the culled tiles used for prefix/postfix and in-row greedier merging
				size_t barrier = delegates.size(); // needed so we know which delegates were added

				// make everything point to the empty delegate by default
				memset(front, 0, sizeof(front));

				// now generate the next row delegates
				for (int b = 0; b < Chunk::size; b ++) {
					uint16_t sprite = plane.at(a, b);

					if (sprite == culled_tile) {
						culled ++;

						// we can connect with culling separated quad only in greedy rows mode
						// otherwise once we hit a culling index we need to brake the quad chain
						if constexpr (!greedier_rows) {
							prev = 0;
						}

						continue;
					} else if (sprite != empty_tile) {
						QuadDelegate& quad = delegates[prev];

						// can we merge with the previous quad?
						if (sprite == quad.sprite && culled < (greedier_culling_limit + 1)) {
							quad.streak ++;

							if constexpr (greedier_rows) {
								quad.streak += culled;
							}

							culled = 0;
							continue;
						}

						int taken = culled;

						// if there is a previous quad share the culled tiles
						if (prev) {
							int given = culled / 2;
							taken = culled - given;

							QuadDelegate& previous = delegates[prev];
							previous.suffix = given;
						}

						delegates.emplace_back(b, sprite, 1, taken, 0);
						prev = delegates.size() - 1;
						culled = 0;
						continue;
					}

					// if we got here then we must have hit empty_tile
					// append all the culling to the previous tile if it exists
					if (prev) {
						delegates[prev].suffix = culled;
					}

					culled = 0;
					prev = 0;
				}

				// append culling to last quad if we hit chunk border while culled
				if (culled && prev) {
					delegates[prev].suffix = culled;
				}

				// imprint quads into row buffer
				for (size_t i = barrier; i < delegates.size(); i ++) {
					QuadDelegate& delegate = delegates[i];

					int length = delegate.streak;
					int start = delegate.offset;

					// adjust range to include culled tiles
					if constexpr (greedier_merge) {
						length += delegate.prefix;
						length += delegate.suffix;
						start -= delegate.prefix;
					}

					for (int j = 0; j < length; j ++) {
						front[j + start] = i;
					}
				}

				// at a=0 we don't have two rows yet, only one
				if (a != 0) {

					// merge and emit back row
					forEachQuad(delegates, back, [&] (int i, QuadDelegate& quad) {

						uint32_t next_id = front[i];
						QuadDelegate& next = delegates[next_id];

						if (next.sprite != quad.sprite) {
							emitQuad<normal>(emitter, chunk.x, chunk.y, chunk.z, slice, a - quad.extend, i, quad.extend, quad.streak, quad.sprite, identity);
							return;
						}

						if constexpr (greedier_merge) {
							int quad_left = quad.offset - quad.prefix;
							int quad_right = quad.offset + quad.streak + quad.suffix;
							int next_left = next.offset - next.prefix;
							int next_right = next.offset + next.streak + next.suffix;

							int start = std::min(quad.offset, next.offset);
							int end = std::max(quad.offset + quad.streak, next.offset + next.streak);

							if ((quad_left <= start && next_left <= start) && (quad_right >= end && next_right >= end)) {

								int merged_left = std::max(quad_left, next_left);
								int merged_right = std::min(quad_right, next_right);

								// this is needed as prefix and postfix can shrink
								// without it they would accumulate outside the correct quad range
								for (int k = 0; k < next.streak + next.prefix + next.suffix; k ++) {
									front[k + next.offset - next.prefix] = 0;
								}

								next.extend = quad.extend + 1;
								next.streak = end - start;
								next.prefix = start - merged_left;
								next.suffix = merged_right - end;
								next.offset = start;

								// imprint adjusted quad size, we don't need to imprint prefix and postfix here
								// as front will now become back, and we need those extended imprints only in front row
								for (int k = 0; k < next.streak; k ++) {
									front[start + k] = next_id;
								}

								return;
							}
						} else {

							// fallback fast-path if greedier_merge is not enabled
							// warning: this only works if the culled tiles are NOT imprinted into rows
							// so it cannot be used while greedier_merge is enabled
							if (next.offset == quad.offset && next.streak == quad.streak) {
								quad.extend ++;
								std::swap(quad, next);
								return;
							}
						}

						emitQuad<normal>(emitter, chunk.x, chunk.y, chunk.z, slice, a - quad.extend, quad.offset, quad.extend, quad.streak, quad.sprite, identity);
					});

				}

				std::swap(back, front);
			}

			// emit the trailing row
			forEachQuad(delegates, back, [&] (int i, QuadDelegate& quad) {
				emitQuad<normal>(emitter, chunk.x, chunk.y, chunk.z, slice, Chunk::size - quad.extend, i, quad.extend, quad.streak, quad.sprite, identity);
			});
		}

	private:

		/**
		 * Imprints the sprite faces into the passed ChunkFaceBuffer
		 * at the given detail level for later used during meshing.
		 */
		static void emitLevel(ChunkFaceBuffer& buffer, WorldView& view, const SpriteArray& array, int level);

	public:

		/**
		 * Emits the chunk mesh into the given buffer, this method will first iterate the chunk
		 * block by block and add all block face sprites into the given ChunkFaceBuffer, after that
		 * is done it will iterate that buffer plane by plane and greedily mesh each chunk slice.
		 *
		 * @param mesh the buffer for the resulting chunk geometry
		 * @param buffer a temporary chunk buffer used during the meshing
		 * @param view access to surrounding chunks
		 * @param array the block sprite storage
		 */
		static void emitChunk(MeshEmitterSet& emitters, ChunkFaceBuffer& buffer, WorldView& view, const SpriteArray& array);

};

// make sure there is no fancy padding added, we rely on the exact memory layout of this thing
static_assert(sizeof(ChunkPlane) == Chunk::size * Chunk::size * sizeof(uint16_t));