
#include "queue.hpp"

/*
 * TaskQueue
 */

TaskQueue::TaskQueue(const TaskQueue& other)
: tasks(other.tasks) {}

TaskQueue::TaskQueue(TaskQueue&& other) noexcept
: tasks(std::move(other.tasks)) {}

void TaskQueue::enqueue(const Task& task) {
	std::lock_guard lock(queue_mutex);
	tasks.emplace(task);
}

int TaskQueue::execute() {
	std::vector<Task> locals;
	locals.reserve(tasks.size());
	int count = tasks.size();

	{
		std::lock_guard lock(queue_mutex);

		while (!tasks.empty()) {
			locals.emplace_back(std::move(tasks.front()));
			tasks.pop();
		}
	}

	for (Task& task : locals) {
		task();
	}

	return count;
}