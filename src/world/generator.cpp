
#include "generator.hpp"

WorldGenerator::WorldGenerator(size_t seed)
: noise(seed) {}

Chunk* WorldGenerator::get(glm::ivec3 pos) {
	const float noise_scale = 16.0f;
	const int max_height = 60;

	Chunk* chunk = new Chunk(pos);

	for (int x = 0; x < Chunk::size; x++) {
		for (int z = 0; z < Chunk::size; z++) {
			int xpos = pos.x * Chunk::size + x;
			int ypos = pos.y * Chunk::size;
			int zpos = pos.z * Chunk::size + z;

			float multiplier = noise.noise3D_01(xpos / 128.0f, zpos / 128.0f, 2137);
			int height = (16 * multiplier + noise.octave2D_01(xpos / noise_scale, zpos / noise_scale, 2) * max_height * multiplier) - max_height * 0.5f;

			if (ypos < height) {
				int local_height = std::min(Chunk::size, height - ypos);
				for (int y = 0; y < local_height; y++) {

					float block_noise = noise.octave3D_01(xpos / noise_scale, (ypos + y) / noise_scale, zpos / noise_scale, 3);

					if (block_noise < 0.7) {
						if (y + ypos < height - 2) {
							chunk->setBlock(x, y, z, Block{1}); // stone
						} else {
							if (block_noise < ((1 - multiplier) * 0.3 + 0.5)) {
								chunk->setBlock(x, y, z, Block{2}); // grass
							}
						}
					}
				}
			}
		}
	}

	return chunk;
}