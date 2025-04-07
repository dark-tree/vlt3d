
#include "task.hpp"
#include "util/logger.hpp"

/*
 * ManagedTask
 */

ManagedTask::ManagedTask(const Task& task)
: task(task), timer() {}

void ManagedTask::call() const {
	double millis = timer.milliseconds();

	if (millis > 200) {
		logger::warn("Is the system overloaded? Task waited ", millis, "ms before starting execution!");
	}

	task();
}