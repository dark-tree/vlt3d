#pragma once

struct Chunk {

	int x, y, z;
	uint32_t blocks[32 * 32 * 32] = {0};

	Chunk(int x, int y, int z)
	: x(x), y(y), z(z) {}

	void random(int count) {
		while (count --> 0) {
			int x = rand() % 32;
			int y = rand() % 32;
			int z = rand() % 32;

			getBlock(x, y, z) = rand();
		}
	}

	inline uint32_t& getBlock(int x, int y, int z) {
		return blocks[(x * 32 * 32) + (y * 32) + (z)];
	}

};

std::vector<Vertex> mesh;

void drawCube(float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite) {
	if (west) {
		mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
		mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v1);
	}

	if (east) {
		mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v2);
		mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
	}

	if (north) {
		mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
		mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
	}

	if (south) {
		mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
		mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
	}

	if(up) {
		mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + -0.5, y + 0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
		mesh.emplace_back(x + -0.5, y + 0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + 0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + 0.5, y + 0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
	}

	if (down) {
		mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + -0.5, y + -0.5, z + 0.5, r, g, b, sprite.u1, sprite.v2);
		mesh.emplace_back(x + -0.5, y + -0.5, z + -0.5, r, g, b, sprite.u1, sprite.v1);
		mesh.emplace_back(x + 0.5, y + -0.5, z + 0.5, r, g, b, sprite.u2, sprite.v2);
		mesh.emplace_back(x + 0.5, y + -0.5, z + -0.5, r, g, b, sprite.u2, sprite.v1);
	}
}

void drawBlock(int x, int y, int z, uint32_t block, bool up, bool down, bool north, bool south, bool west, bool east, Atlas& atlas) {
	if (block == 0) {
		return;
	}

	float r = (block & 0xFF) / 255.0f;
	float g = (block >> 8 & 0xFF) / 255.0f;
	float b = (block >> 16 & 0xFF) / 255.0f;
	bool sprite = (block >> 24 & 0xFF) & 0b0010000;

	drawCube(x, y, z, r, g, b, up, down, north, south, west, east, sprite ? atlas.getSprite("assets/sprites/vkblob.png") : atlas.getSprite("assets/sprites/digital.png"));
}

void drawChunk(Chunk& chunk, Atlas& atlas) {
	for (int x = 0; x < 32; x ++) {
		for (int y = 0; y < 32; y ++) {
			for (int z = 0; z < 32; z ++) {
				uint32_t block = chunk.getBlock(x, y, z);

				const int px = x - 1;
				const int nx = x + 1;
				const int py = y - 1;
				const int ny = y + 1;
				const int pz = z - 1;
				const int nz = z + 1;

				drawBlock(chunk.x * 32 + x, chunk.y * 32 + y, chunk.z * 32 + z, block,
						  ny > 31 || (chunk.getBlock(x, ny, z) == 0),
						  py < 00 || (chunk.getBlock(x, py, z) == 0),
						  nx > 31 || (chunk.getBlock(nx, y, z) == 0), // TODO would be nice if those two were cached in
						  px < 00 || (chunk.getBlock(px, y, z) == 0), //      the actual block being culled for performance
						  nz > 31 || (chunk.getBlock(x, y, nz) == 0),
						  pz < 00 || (chunk.getBlock(x, y, pz) == 0),
						  atlas
				);
			}
		}
	}
}