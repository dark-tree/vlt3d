#pragma once

#include "external.hpp"

class Random {

	private:

		std::mt19937 random;

	public:

		Random() = default;
		Random(size_t seed);

	public:

		/**
		 * Sets the seed of this pseudo random number generator to
		 * the provided integer, resets the number sequence
		 */
		void useSeed(size_t seed);

		/**
		 * Uses a hardware entropy source to seed this
		 * pseudo random number generator
		 */
		void useHardwareSeed();

		float uniformFloat(float min, float max);
		float uniformFloat(float max);
		int uniformInt(int min, int max);
		int uniformInt(int max);
		float gaussianFloat(float mean, float deviation);

};
