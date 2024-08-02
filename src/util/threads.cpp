
#include "threads.hpp"

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

/*
 * TaskPool
 */

void TaskPool::run() {

	ManagedTask task;
	while (true) {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// wait for task to appear in tasks queue or for the stop sequance to begin
			condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
			if (stop && tasks.empty()) return;

			task = std::move(tasks.front());
			tasks.pop();
		}

		task.call();
	}

}

size_t TaskPool::optimal() {
	return std::max((int) std::thread::hardware_concurrency() - 1, 1);
}

TaskPool::TaskPool(size_t count)
: stop(false) {
	logger::info("Created thread pool ", this, " of size: ", count);

	while (count --> 0) {
		workers.emplace_back(&TaskPool::run, this);
	}
}

TaskPool::~TaskPool() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}

	this->condition.notify_all();

	for (auto& worker : this->workers) {
		worker.join();
	}

	logger::info("Stopped thread pool ", this);
}

void TaskPool::enqueue(const Task& task) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		// don't allow enqueueing after stopping the pool
		if (stop) {
			throw std::runtime_error("Unable to add task to a stopped pool!");
		}

		tasks.emplace(task);
	}

	condition.notify_one();
}