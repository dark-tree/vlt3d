
#include "mesher.hpp"
#include "renderer.hpp"
#include "world/view.hpp"

/*
 * ChunkPlane
 */

uint16_t& ChunkPlane::at(int alpha, int beta) {
	return faces[beta + alpha * Chunk::size];
}

/*
 * ChunkFaceBuffer
 */

ChunkPlane& ChunkFaceBuffer::get(int value, int offset) {
	return buffer[planes_per_axis * value + offset];
}

ChunkFaceBuffer::ChunkFaceBuffer() {
	this->buffer = new ChunkPlane[size];
}

ChunkFaceBuffer::~ChunkFaceBuffer() {
	delete[] this->buffer;
}

void ChunkFaceBuffer::clear(uint16_t empty) {
	memset(this->buffer, empty, size * sizeof(ChunkPlane));
}

ChunkPlane& ChunkFaceBuffer::getX(int x, int offset) {
	return get(x, offset);
}

ChunkPlane& ChunkFaceBuffer::getY(int y, int offset) {
	return get(y, offset + 1 * planes_along_axis * planes_per_axis);
}

ChunkPlane& ChunkFaceBuffer::getZ(int z, int offset) {
	return get(z, offset + 2 * planes_along_axis * planes_per_axis);
}

BlockFaceView ChunkFaceBuffer::getBlockView(int x, int y, int z) {
	BlockFaceView view {};

	view.west = &getX(x, 0).at(y, z);
	view.east = &getX(x, 1).at(y, z);
	view.down = &getY(y, 0).at(x, z);
	view.up = &getY(y, 1).at(x, z);
	view.north = &getZ(z, 0).at(x, y);
	view.south = &getZ(z, 1).at(x, y);

	return view;
}

/*
 * GreedyMesher
 */

void GreedyMesher::emitLevel(ChunkFaceBuffer& buffer, WorldView& view, const SpriteArray& array, int level) {

	buffer.clear(GreedyMesher::empty_tile);

	int gray_sprite = array.getSpriteIndex("gray");
	int clay_sprite = array.getSpriteIndex("clay");
	int moss_sprite = array.getSpriteIndex("moss");
	int side_sprite = array.getSpriteIndex("side");

	int mask = 0;

	for (int i = 1; i < level; i <<= 1) {
		mask |= i;
	}

	const auto fetchBlock = [mask = ~mask, &view] (int x, int y, int z) -> Block {
		return view.getBlock(x & mask, y & mask, z & mask);
	};

	glm::ivec3 offset = view.origin() * Chunk::size;

	for (int z = 0; z < Chunk::size; z++) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {

				glm::ivec3 pos = offset + glm::ivec3 {x, y, z};
				Block block = fetchBlock(pos.x, pos.y, pos.z);

				if (block.isAir()) {
					continue;
				}

				BlockFaceView faces = buffer.getBlockView(x, y, z);

				int top = (block.block_type % 2 == 1) ? gray_sprite : clay_sprite;
				int side = top;
				int bottom = top;

				bool west = fetchBlock(pos.x - 1, pos.y, pos.z).isAir();
				bool east = fetchBlock(pos.x + 1, pos.y, pos.z).isAir();
				bool down = fetchBlock(pos.x, pos.y - 1, pos.z).isAir();
				bool up = fetchBlock(pos.x, pos.y + 1, pos.z).isAir();
				bool north = fetchBlock(pos.x, pos.y, pos.z - 1).isAir();
				bool south = fetchBlock(pos.x, pos.y, pos.z + 1).isAir();

				if (bottom == clay_sprite && up) {
					side = side_sprite;
					top = moss_sprite;
				}

				*faces.west = west ? side : culled_tile;
				*faces.east = east ? side : culled_tile;
				*faces.down = down ? bottom : culled_tile;
				*faces.up = up ? top : culled_tile;
				*faces.north = north ? side : culled_tile;
				*faces.south = south ? side : culled_tile;
			}
		}
	}

}

void GreedyMesher::emitChunk(MeshEmitterSet& emitters, ChunkFaceBuffer& buffer, WorldView& view, const SpriteArray& array) {

	emitLevel(buffer, view, array, 1);
	glm::ivec3 offset = view.origin() * Chunk::size;

	// this can be done on 3 threads if we need more speed
	for (int slice = 0; slice < Chunk::size; slice ++) {
		emitPlane<Normal::WEST>(emitters.get(DirectionIndex::WEST), offset, slice, buffer.getX(slice, 0));
		emitPlane<Normal::EAST>(emitters.get(DirectionIndex::EAST), offset, slice, buffer.getX(slice, 1));
		emitPlane<Normal::DOWN>(emitters.get(DirectionIndex::DOWN), offset, slice, buffer.getY(slice, 0));
		emitPlane<Normal::UP>(emitters.get(DirectionIndex::UP), offset, slice, buffer.getY(slice, 1));
		emitPlane<Normal::NORTH>(emitters.get(DirectionIndex::NORTH), offset, slice, buffer.getZ(slice, 0));
		emitPlane<Normal::SOUTH>(emitters.get(DirectionIndex::SOUTH), offset, slice, buffer.getZ(slice, 1));
	}

	emitLevel(buffer, view, array, 2);

	// this can be done on 3 threads if we need more speed
	for (int slice = 0; slice < Chunk::size; slice ++) {
		emitPlane<Normal::WEST>(emitters.get(MeshEmitterSet::LOD_2), offset, slice, buffer.getX(slice, 0));
		emitPlane<Normal::EAST>(emitters.get(MeshEmitterSet::LOD_2), offset, slice, buffer.getX(slice, 1));
		emitPlane<Normal::DOWN>(emitters.get(MeshEmitterSet::LOD_2), offset, slice, buffer.getY(slice, 0));
		emitPlane<Normal::UP>(emitters.get(MeshEmitterSet::LOD_2), offset, slice, buffer.getY(slice, 1));
		emitPlane<Normal::NORTH>(emitters.get(MeshEmitterSet::LOD_2), offset, slice, buffer.getZ(slice, 0));
		emitPlane<Normal::SOUTH>(emitters.get(MeshEmitterSet::LOD_2), offset, slice, buffer.getZ(slice, 1));
	}

}