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

		uint32_t& ref(int x, int y, int z) {
			return (*blocks)[x + y * size + z * size * size];
		}

	public:

		int cx, cy, cz;

		Chunk(glm::ivec3 pos)
		: cx(pos.x), cy(pos.y), cz(pos.z) {}

		~Chunk() {
			delete[] blocks;
		}

		void setBlock(int x, int y, int z, uint32_t block) {
			if (!blocks) {
				blocks = (uint32_t (*)[size * size * size]) std::calloc(size * size * size, sizeof(uint32_t));
			}

			ref(x, y, z) = block;
		}

		uint32_t getBlock(int x, int y, int z) {
			if (!blocks) {
				return 0;
			}

			return ref(x, y, z);
		}

		bool isEmpty() {
			return blocks == nullptr;
		}

};
