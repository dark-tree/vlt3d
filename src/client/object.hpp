#pragma once

#include "external.hpp"

class WorldObject {

	public:

		virtual ~WorldObject() = default;

		virtual glm::vec3 getPosition() const = 0;
		virtual glm::vec3 getDirection() const = 0;

		float distance(const WorldObject& other) const;
		float distance(glm::vec3 other) const;


};
