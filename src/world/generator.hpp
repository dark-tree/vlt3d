#pragma once

#include "external.hpp"
#include "chunk.hpp"

class WorldGenerator {

	private:

		siv::PerlinNoise noise;

	public:

		WorldGenerator(size_t seed);

		/// Generates (as in world generation, not meshing) the requested chunk and returns it
		Chunk* get(glm::ivec3 pos);

};
