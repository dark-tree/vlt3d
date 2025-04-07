#pragma once

#include "external.hpp"
#include "task.hpp"

/**
 * This class can be used to execute callbacks on
 * specific threads or in specific places
 */
class TaskQueue {

	private:

		std::mutex queue_mutex;
		std::queue<Task> tasks;

	public:

		TaskQueue() = default;
		TaskQueue(const TaskQueue& other);
		TaskQueue(TaskQueue&& other) noexcept;

		void enqueue(const Task& task);

		template <typename Func, typename Arg, typename... Args>
		void enqueue(Func func, Arg arg, Args... args) {
			enqueue(std::bind(func, arg, args...));
		}

	public:

		/**
		 * Execute all pending task in this queue
		 */
		int execute();

};