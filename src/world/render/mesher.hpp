#pragma once

#include "external.hpp"
#include "client/vertices.hpp"
#include "world/chunk.hpp"
#include "buffer/sprites.hpp"

class WorldRenderView;
class SpriteArray;

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

		struct QuadDelegate {
			uint16_t offset;
			uint16_t index;
			uint16_t streak;
			uint16_t extend;

			// check if the two quads can be merged together (along extend)
			bool canMergeWith(const QuadDelegate& other) const;
		};

		template <typename Func>
		static void forEachQuad(std::vector<QuadDelegate>& delegates, uint32_t row[], Func func) {
			for (int i = 0; i < Chunk::size;) {
				QuadDelegate& quad = delegates[row[i]];

				if (quad.index) {
					func(i, quad);
				}

				i += quad.streak;
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

					// merge and emit back row
					forEachQuad(delegates, back, [&] (int i, QuadDelegate& quad) {
						QuadDelegate& next = delegates[front[i]];

						if (quad.canMergeWith(next)) {
							quad.extend ++;
							std::swap(quad, next);
						} else {
							emitQuad<normal>(mesh, chunk.x, chunk.y, chunk.z, slice, a - quad.extend, i, quad.extend, quad.streak, quad.index, sprite);
						}
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

		static void emitChunk(std::vector<VertexTerrain>& mesh, ChunkFaceBuffer& buffer, std::shared_ptr<Chunk> chunk, WorldRenderView& view, const SpriteArray& array);

};

// make sure there is no fancy padding added, we rely on the exact memory layout of this thing
static_assert(sizeof(ChunkPlane) == Chunk::size * Chunk::size * sizeof(uint16_t));