#pragma once

#include "external.hpp"
#include "util/ring.hpp"

class Profiler {

	private:

		double time;
		int count;
		int result;
		RingBuffer<double, 32> running;
		RingBuffer<double, 256> history;

	public:

		Profiler();

		void next();
		void addFrameTime(double millis);

		int getCountPerSecond();
		double getAvgFrameTime();
		double getMaxFrameTime();
		std::add_lvalue_reference<decltype(history)>::type getAvgFrameTimeHistory();

};
