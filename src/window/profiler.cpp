
#include "profiler.hpp"

Profiler::Profiler() {
	this->count = 0;
	this->result = 0;
	this->time = glfwGetTime();
}

void Profiler::next() {
	double current = glfwGetTime();
	this->count ++;

	if (current - time >= 1.0) {
		this->result = this->count;
		this->count = 0;

		this->time = current;
	}
}

int Profiler::getCountPerSecond() {
	return this->result;
}