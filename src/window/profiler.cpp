
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

void Profiler::addFrameTime(double millis) {
	history.insert(millis);
	running.insert(millis);
}

int Profiler::getCountPerSecond() {
	return this->result;
}

double Profiler::getAvgFrameTime() {
	return std::reduce(running.begin(), running.end()) / running.size();
}

double Profiler::getMaxFrameTime() {
	return *std::max_element(running.begin(), running.end());
}

auto Profiler::getAvgFrameTimeHistory() -> std::add_lvalue_reference<decltype(history)>::type {
	return history;
}