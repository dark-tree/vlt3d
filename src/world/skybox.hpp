#pragma once

#include "external.hpp"
#include "client/immediate.hpp"
#include "util/random.hpp"

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

		void draw(ImmediateRenderer& immediate, Camera& camera) const;

};
