
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

void GreedyMesher::emitChunk(MeshEmitterSet& emitters, ChunkFaceBuffer& buffer, WorldView& view, const SpriteArray& array) {

	buffer.clear(GreedyMesher::empty_tile);
	Chunk* self = view.getOriginChunk();

	int gray_sprite = array.getSpriteIndex("gray");
	int clay_sprite = array.getSpriteIndex("clay");
	int moss_sprite = array.getSpriteIndex("moss");
	int side_sprite = array.getSpriteIndex("side");

	glm::ivec3 offset = view.origin() * Chunk::size;

	for (int z = 0; z < Chunk::size; z++) {
		for (int y = 0; y < Chunk::size; y++) {
			for (int x = 0; x < Chunk::size; x++) {
				Block block = self->getBlock(x, y, z);

				if (block.isAir()) {
					continue;
				}

				BlockFaceView faces = buffer.getBlockView(x, y, z);
				glm::ivec3 pos = offset + glm::ivec3 {x, y, z};

				int top = (block.block_type % 2 == 1) ? gray_sprite : clay_sprite;
				int side = top;
				int bottom = top;

				bool west = view.getBlock(pos.x - 1, pos.y, pos.z).isAir();
				bool east = view.getBlock(pos.x + 1, pos.y, pos.z).isAir();
				bool down = view.getBlock(pos.x, pos.y - 1, pos.z).isAir();
				bool up = view.getBlock(pos.x, pos.y + 1, pos.z).isAir();
				bool north = view.getBlock(pos.x, pos.y, pos.z - 1).isAir();
				bool south = view.getBlock(pos.x, pos.y, pos.z + 1).isAir();

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

	// this can be done on 3 threads if we need more speed
	for (int slice = 0; slice < Chunk::size; slice ++) {
		emitPlane<Normal::WEST>(emitters.getWest(), offset, slice, buffer.getX(slice, 0));
		emitPlane<Normal::EAST>(emitters.getEast(), offset, slice, buffer.getX(slice, 1));
		emitPlane<Normal::DOWN>(emitters.getDown(), offset, slice, buffer.getY(slice, 0));
		emitPlane<Normal::UP>(emitters.getUp(), offset, slice, buffer.getY(slice, 1));
		emitPlane<Normal::NORTH>(emitters.getNorth(), offset, slice, buffer.getZ(slice, 0));
		emitPlane<Normal::SOUTH>(emitters.getSouth(), offset, slice, buffer.getZ(slice, 1));
	}

}