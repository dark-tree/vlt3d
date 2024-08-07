#pragma once

#include "external.hpp"
#include "client/immediate.hpp"
#include "util/random.hpp"

struct Sun {
	glm::vec3 pos;
	uint8_t r, g, b, v;
};

class Skybox {

	private:

		static constexpr float SUN_SPEED = glm::radians(15.0f); // radians per hour
		static constexpr float AXIAL_TILT = glm::radians(23.44f);

	private:

		/// Returns Sun's Equatorial Coordinates at given time
		glm::vec2 getSun(float hour, float day) const;

		/// Translates Equatorial Coordinates to observer's Horizontal Coordinates
		glm::vec2 toHorizontal(glm::vec2 equatorial, float latitude) const;

		void drawSphere(ImmediateRenderer& immediate, glm::vec3 pos, float radius, int longs, int lats, BakedSprite sprite) const;

	public:

		glm::vec3 getSunPos(float observer_latitude) const;

		inline Sun getSunData(float observer_latitude) const {
			Sun sun;
			sun.pos = getSunPos(observer_latitude);
			int s = std::max(0, (int) -(sun.pos.y * 200));

			sun.r = std::max(0, 155 - (int) s);
			sun.g = std::max(0, 50 - (int) s);
			sun.b = std::max(0, 50 - (int) s);
			return sun;
		}

		void draw(ImmediateRenderer& immediate, Camera& camera) const;

};
