
#include "external.hpp"

class VoronoiNoise {

	private:

		float noise(int x, int y, int z) {
			uint32_t seed = uint32_t(x) * 1087;
			seed ^= 0xE56FAA12;
			seed += uint32_t(y) * 2749;
			seed ^= 0x69628a2d;
			seed += uint32_t(z) * 3433;
			seed ^= 0xa7b2c49a;

			return (float) (seed % 1000) / 1000.0f;
		}

		glm::vec3 chunk(int x, int y, int z) {
			return {x + noise(x, y, z), y + noise(z + 53, x + 197, y + 967), z + noise(y + 5, z + 829, x + 541)};
		}

	public:

		float get(float x, float y, float z) {
			glm::vec3 sample {x, y, z};

			int bx = (int) std::floor(x);
			int by = (int) std::floor(y);
			int bz = (int) std::floor(z);

			float value = std::numeric_limits<float>::max();

			for (int cx = -1; cx <= 1; cx ++) {
				for (int cy = -1; cy <= 1; cy ++) {
					for (int cz = -1; cz <= 1; cz ++) {
						float dist = glm::distance(chunk(bx + cx, by + cy, bz + cz), sample);

						if (dist < value) {
							value = dist;
						}
					}
				}
			}

			return value;
		}

};