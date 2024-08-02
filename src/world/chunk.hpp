#pragma once

#include "external.hpp"

class Chunk {

	public:

		static constexpr int size = 0b100000;
		static constexpr int mask = size - 1;
		static constexpr int bits = std::popcount((size_t) mask);

		static_assert(std::popcount((size_t) size) == 1, "Chunk::size needs to be a power of 2");

	private:

		uint32_t (*blocks)[size * size * size] = nullptr;

		uint32_t& ref(int x, int y, int z);

	public:

		READONLY glm::ivec3 pos;

		Chunk(glm::ivec3 pos);
		~Chunk();

		/// Sets a block at the given chunk position
		void setBlock(int x, int y, int z, uint32_t block);

		/// Gets the block at the given chunk position
		uint32_t getBlock(int x, int y, int z);

		/// Checks if this chunks contains no blocks in O(1) time
		bool empty();

};
