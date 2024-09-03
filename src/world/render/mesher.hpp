#pragma once

#include "external.hpp"
#include "client/vertices.hpp"
#include "world/chunk.hpp"
#include "buffer/sprites.hpp"

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

		void clear();

		ChunkPlane& getX(int x, int offset);
		ChunkPlane& getY(int y, int offset);
		ChunkPlane& getZ(int z, int offset);

		BlockFaceView getBlockView(int x, int y, int z);

};

class GreedyMesher {

	private:

		static constexpr bool greedier_rows = true;
		static constexpr bool greedier_merge = true;
		static constexpr int greedier_culling_limit = 6;

		struct QuadDelegate {
			uint16_t offset;
			uint16_t index;
			uint16_t streak;
			uint16_t extend;
			uint16_t prefix;
			uint16_t suffix;

			QuadDelegate(uint16_t offset, uint16_t sprite, uint16_t streak, uint16_t prefix, uint16_t suffix)
			: offset(offset), index(sprite), streak(streak), extend(1), prefix(prefix), suffix(suffix) {}
		};

		template <typename Func>
		static void forEachQuad(std::vector<QuadDelegate>& delegates, uint32_t row[], Func func) {
			for (int i = 0; i < Chunk::size;) {
				QuadDelegate& quad = delegates[row[i]];

				if (quad.index) {
					// `i` and `quad.offset` are not always the same value
					// as prefix and postfix can also be imprinted in the row
					i = quad.offset;
					func(i, quad);
				}

				i += quad.streak;
				i += quad.suffix;
			}
		}

	private:

		template <Normal normal>
		static void emitQuad(std::vector<VertexTerrain>& mesh, float x, float y, float z, float slice, float alpha, float beta, float width, float height, uint16_t index, BakedSprite sprite) {

			const float aps = -(alpha - 0.5f);
			const float bps = -(beta - 0.5f);
			const float apo = width - aps;
			const float bpo = height - bps;

			BakedSprite swh {sprite.u1, sprite.v1, sprite.u2 * width, sprite.v2 * height};
			BakedSprite shw {sprite.u1, sprite.v1, sprite.u2 * height, sprite.v2 * width};

			if constexpr (normal == Normal::EAST) {
				x += slice;

				mesh.emplace_back(x + 0.5, y - aps, z - bps, shw.u1, shw.v2, index, 255, 0, 0, Normal::EAST);
				mesh.emplace_back(x + 0.5, y + apo, z + bpo, shw.u2, shw.v1, index, 0, 255, 0, Normal::EAST);
				mesh.emplace_back(x + 0.5, y - aps, z + bpo, shw.u2, shw.v2, index, 0, 0, 255, Normal::EAST);

				mesh.emplace_back(x + 0.5, y - aps, z - bps, shw.u1, shw.v2, index, 255, 0, 0, Normal::EAST);
				mesh.emplace_back(x + 0.5, y + apo, z - bps, shw.u1, shw.v1, index, 0, 255, 0, Normal::EAST);
				mesh.emplace_back(x + 0.5, y + apo, z + bpo, shw.u2, shw.v1, index, 0, 0, 255, Normal::EAST);
			}

			if constexpr (normal == Normal::WEST) {
				x += slice;

				mesh.emplace_back(x - 0.5, y - aps, z - bps, shw.u1, shw.v2, index, 255, 0, 0, Normal::WEST);
				mesh.emplace_back(x - 0.5, y - aps, z + bpo, shw.u2, shw.v2, index, 0, 255, 0, Normal::WEST);
				mesh.emplace_back(x - 0.5, y + apo, z + bpo, shw.u2, shw.v1, index, 0, 0, 255, Normal::WEST);

				mesh.emplace_back(x - 0.5, y - aps, z - bps, shw.u1, shw.v2, index, 255, 0, 0, Normal::WEST);
				mesh.emplace_back(x - 0.5, y + apo, z + bpo, shw.u2, shw.v1, index, 0, 255, 0, Normal::WEST);
				mesh.emplace_back(x - 0.5, y + apo, z - bps, shw.u1, shw.v1, index, 0, 0, 255, Normal::WEST);
			}

			if constexpr (normal == Normal::UP) {
				y += slice;

				mesh.emplace_back(x - aps, y + 0.5, z - bps, swh.u1, swh.v1, index, 255, 0, 0, Normal::UP);
				mesh.emplace_back(x - aps, y + 0.5, z + bpo, swh.u1, swh.v2, index, 0, 255, 0, Normal::UP);
				mesh.emplace_back(x + apo, y + 0.5, z + bpo, swh.u2, swh.v2, index, 0, 0, 255, Normal::UP);

				mesh.emplace_back(x - aps, y + 0.5, z - bps, swh.u1, swh.v1, index, 255, 0, 0, Normal::UP);
				mesh.emplace_back(x + apo, y + 0.5, z + bpo, swh.u2, swh.v2, index, 0, 255, 0, Normal::UP);
				mesh.emplace_back(x + apo, y + 0.5, z - bps, swh.u2, swh.v1, index, 0, 0, 255, Normal::UP);
			}

			if constexpr (normal == Normal::DOWN) {
				y += slice;

				mesh.emplace_back(x - aps, y - 0.5, z - bps, swh.u1, swh.v1, index, 255, 0, 0, Normal::DOWN);
				mesh.emplace_back(x + apo, y - 0.5, z + bpo, swh.u2, swh.v2, index, 0, 255, 0, Normal::DOWN);
				mesh.emplace_back(x - aps, y - 0.5, z + bpo, swh.u1, swh.v2, index, 0, 0, 255, Normal::DOWN);

				mesh.emplace_back(x - aps, y - 0.5, z - bps, swh.u1, swh.v1, index, 255, 0, 0, Normal::DOWN);
				mesh.emplace_back(x + apo, y - 0.5, z - bps, swh.u2, swh.v1, index, 0, 255, 0, Normal::DOWN);
				mesh.emplace_back(x + apo, y - 0.5, z + bpo, swh.u2, swh.v2, index, 0, 0, 255, Normal::DOWN);
			}

			if constexpr (normal == Normal::SOUTH) {
				z += slice;

				mesh.emplace_back(x - aps, y - bps, z + 0.5, swh.u1, swh.v2, index, 255, 0, 0, normal);
				mesh.emplace_back(x + apo, y + bpo, z + 0.5, swh.u2, swh.v1, index, 0, 255, 0, normal);
				mesh.emplace_back(x - aps, y + bpo, z + 0.5, swh.u1, swh.v1, index, 0, 0, 255, normal);

				mesh.emplace_back(x - aps, y - bps, z + 0.5, swh.u1, swh.v2, index, 255, 0, 0, normal);
				mesh.emplace_back(x + apo, y - bps, z + 0.5, swh.u2, swh.v2, index, 0, 255, 0, normal);
				mesh.emplace_back(x + apo, y + bpo, z + 0.5, swh.u2, swh.v1, index, 0, 0, 255, normal);
			}

			if constexpr (normal == Normal::NORTH) {
				z += slice;

				mesh.emplace_back(x - aps, y - bps, z - 0.5, swh.u1, swh.v2, index, 255, 0, 0, Normal::NORTH);
				mesh.emplace_back(x - aps, y + bpo, z - 0.5, swh.u1, swh.v1, index, 0, 255, 0, Normal::NORTH);
				mesh.emplace_back(x + apo, y + bpo, z - 0.5, swh.u2, swh.v1, index, 0, 0, 255, Normal::NORTH);

				mesh.emplace_back(x - aps, y - bps, z - 0.5, swh.u1, swh.v2, index, 255, 0, 0, Normal::NORTH);
				mesh.emplace_back(x + apo, y + bpo, z - 0.5, swh.u2, swh.v1, index, 0, 255, 0, Normal::NORTH);
				mesh.emplace_back(x + apo, y - bps, z - 0.5, swh.u2, swh.v2, index, 0, 0, 255, Normal::NORTH);
			}

		}

		template <Normal normal>
		static void emitPlane(std::vector<VertexTerrain>& mesh, glm::ivec3 chunk, int slice, ChunkPlane& plane) {
			const BakedSprite sprite = BakedSprite::identity();

			std::vector<QuadDelegate> delegates;
			delegates.emplace_back(0, 0, 1, 0, 0);

			uint32_t back[Chunk::size];
			uint32_t front[Chunk::size];

			for (int a = 0; a < Chunk::size; a ++) {
				size_t prev = 0;
				uint16_t culled = 0;
				int barrier = delegates.size();

				memset(front, 0, sizeof(front));

				for (int b = 0; b < Chunk::size; b ++) {
					uint16_t index = plane.at(a, b);

					if (index == 0xFFFF) {
						culled ++;

						// we can connect with culling separated quad only in greedy rows mode
						// otherwise once we hit a culling index we need to brake the quad chain
						if constexpr (!greedier_rows) {
							prev = 0;
						}

						continue;
					} else if (index) {
						QuadDelegate& quad = delegates[prev];

						if (index == quad.index && culled < (greedier_culling_limit + 1)) {
							quad.streak ++;

							if constexpr (greedier_rows) {
								quad.streak += culled;
							}

							culled = 0;
							continue;
						}

						int taken = culled;

						if (prev) {
							int given = culled / 2;
							taken = culled - given;

							QuadDelegate& previous = delegates[prev];
							previous.suffix = given;
						}

						delegates.emplace_back(b, index, 1, taken, 0);
						prev = delegates.size() - 1;
						culled = 0;
						continue;
					}

					if (prev) {
						QuadDelegate& previous = delegates[prev];
						previous.suffix = culled;
					}

					culled = 0;
					prev = 0;
				}

				// append culling to last quad if we hit chunk border (exited the loop)
				if (culled && prev) {
					QuadDelegate& previous = delegates[prev];
					previous.suffix = culled;
				}

				// imprint quads into row buffer
				for (int i = barrier; i < delegates.size(); i ++) {
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

				if (a != 0) {

					// merge and emit back row
					forEachQuad(delegates, back, [&] (int i, QuadDelegate& quad) {

						uint32_t next_id = front[i];
						QuadDelegate& next = delegates[next_id];

						if (next.index != quad.index) {
							emitQuad<normal>(mesh, chunk.x, chunk.y, chunk.z, slice, a - quad.extend, i, quad.extend, quad.streak, quad.index, sprite);
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

								for (int k = 0; k < next.streak + next.prefix + next.suffix; k ++) {
									front[k + next.offset - next.prefix] = 0;
								}

								next.extend = quad.extend + 1;
								next.streak = end - start;
								next.prefix = start - merged_left;
								next.suffix = merged_right - end;
								next.offset = start;

								for (int k = 0; k < next.streak; k ++) {
									front[start + k] = next_id;
								}

								return;
							}
						} else {

							// fallback fast-path if greedier_merge is not enabled
							// warning: this only works if the culled tiles are NOT imprinted into rows
							if (next.offset == quad.offset && next.streak == quad.streak) {
								quad.extend ++;
								std::swap(quad, next);
								return;
							}
						}

						emitQuad<normal>(mesh, chunk.x, chunk.y, chunk.z, slice, a - quad.extend, quad.offset, quad.extend, quad.streak, quad.index, sprite);
					});

				}

				std::swap(back, front);
			}

			// emit the trailing row
			forEachQuad(delegates, back, [&] (int i, QuadDelegate& quad) {
				emitQuad<normal>(mesh, chunk.x, chunk.y, chunk.z, slice, Chunk::size - quad.extend, i, quad.extend, quad.streak, quad.index, sprite);
			});
		}

	public:

		static void emitChunk(std::vector<VertexTerrain>& mesh, ChunkFaceBuffer& buffer, WorldView& view, const SpriteArray& array);

};

// make sure there is no fancy padding added, we rely on the exact memory layout of this thing
static_assert(sizeof(ChunkPlane) == Chunk::size * Chunk::size * sizeof(uint16_t));