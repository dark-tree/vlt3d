
#include "generator.hpp"

WorldGenerator::WorldGenerator(size_t seed)
: noise(seed) {}

Chunk* WorldGenerator::get(glm::ivec3 pos) {
	const float noise_scale = 16.0f;
	const int max_height = 32;

	Chunk* chunk = new Chunk(pos);

	for (int x = 0; x < Chunk::size; x++) {
		for (int z = 0; z < Chunk::size; z++) {
			int xpos = pos.x * Chunk::size + x;
			int ypos = pos.y * Chunk::size;
			int zpos = pos.z * Chunk::size + z;

			int height = noise.noise2D_01(xpos / noise_scale, zpos / noise_scale) * max_height - max_height * 0.5f;

			if (ypos < height) {
				int local_height = std::min(Chunk::size, height - ypos);
				for (int y = 0; y < local_height; y++) {

					if (y + ypos < height - 2) {
						chunk->setBlock(x, y, z, Block {1});
					} else {
						chunk->setBlock(x, y, z, Block {2});
					}

				}
			}
		}
	}

	return chunk;
}