#pragma once

#include "external.hpp"

class Profiler {

	private:

		double time;
		int count;
		int result;

	public:

		Profiler();

		void next();
		int getCountPerSecond();

};
