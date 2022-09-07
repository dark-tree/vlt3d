#pragma once

#include <include/external.hpp>

class Uniform {

	private:

		const GLint location;

	public:
	
		Uniform(GLint location);

		GLint getLocation();

		void set(int value);
		void set(float value);
		void set(const glm::vec3& value);
		void set(const glm::mat3& value);
		void set(const glm::mat4& value);
		
};

