
#include "random.hpp"

Random::Random(size_t seed)
: random(seed) {}

void Random::useSeed(size_t seed) {
	random.seed(seed);
}

void Random::useHardwareSeed() {
	static std::random_device random_device;
	random.seed(random_device());
}

float Random::uniformFloat(float min, float max) {
	std::uniform_real_distribution distribution {min, max};
	return distribution(this->random);
}

float Random::uniformFloat(float max) {
	return uniformFloat(0, max);
}

int Random::uniformInt(int min, int max) {
	std::uniform_int_distribution distribution {min, max};
	return distribution(this->random);
}

int Random::uniformInt(int max) {
	return uniformInt(0, max);
}

float Random::gaussianFloat(float mean, float deviation) {
	std::normal_distribution distribution {mean, deviation};
	return distribution(this->random);
}
