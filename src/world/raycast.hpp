#pragma once

#include "external.hpp"
#include "util/direction.hpp"

class Raycast {

	private:

		bool successful;
		glm::ivec3 hit;
		glm::ivec3 last;

	public:

		Raycast();
		Raycast(glm::ivec3 hit, glm::ivec3 last);

		operator bool() const;
		glm::ivec3 getPos() const;
		glm::ivec3 getTarget() const;

};
