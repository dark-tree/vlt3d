#pragma once

#include "external.hpp"
#include "util/type/direction.hpp"
#include "block.hpp"

class Chunk {

	public:

		static constexpr int size = 0b100000;
		static constexpr int mask = size - 1;
		static constexpr int bits = std::popcount((size_t) mask);

		static_assert(std::popcount((size_t) size) == 1, "Chunk::size needs to be a power of 2");

		static Direction::field_type getNeighboursMask(int x, int y, int z);

	private:

		Block (*blocks)[size * size * size] = nullptr;

		Block& ref(int x, int y, int z);

	public:

		READONLY glm::ivec3 pos;

		Chunk(glm::ivec3 pos);
		~Chunk();

		/// Sets a block at the given chunk position
		void setBlock(int x, int y, int z, Block block);

		/// Gets the block at the given chunk position
		Block getBlock(int x, int y, int z);

		/// Checks if this chunks contains no blocks in O(1) time
		bool empty();

};
