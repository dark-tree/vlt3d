#include "uniform.hpp"

Uniform::Uniform(GLint location) : location(location) {

}

GLint Uniform::getLocation() {
	return this->location;
}

void Uniform::set(int value) {
	glUniform1i(location, value);
}

void Uniform::set(float value) {
	glUniform1f(location, value);
}

void Uniform::set(const glm::vec3& value) {
	glUniform3fv(location, 1, glm::value_ptr(value));
}

void Uniform::set(const glm::mat3& value) {
	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Uniform::set(const glm::mat4& value) {
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
