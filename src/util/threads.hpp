#pragma once

#include <utility>

#include "logger.hpp"
#include "timer.hpp"
#include "external.hpp"

using Task = std::function<void()>;

/**
 * A wrapper around a std::function that allows
 * TaskPool to attach additional information to tasks
 */
class ManagedTask {

	private:

		Task task;
		Timer timer;

	public:

		ManagedTask() = default;
		ManagedTask(const Task& task);

		inline void call() const;

};

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

/**
 * A basic thread pool implementation with
 * support for std::futures
 */
class TaskPool {

	private:

		bool stop;

		std::vector<std::thread> workers;
		std::mutex queue_mutex;
		std::queue<ManagedTask> tasks;
		std::condition_variable condition;

		void run();

	public:

		TaskPool(size_t count = TaskPool::optimal());
		~TaskPool();

		/**
		 * Get the "optimal" number of threads for a
		 * thread pool on this hardware
		 */
		static size_t optimal();

		/**
		 * Enqueue a task for execution by one of the threads
		 * on this thread pool, the tasks start execution in a FIFO order
		 */
		void enqueue(const Task& task);

	public:

		template <typename Func, typename Arg, typename... Args>
		void enqueue(Func func, Arg arg, Args... args) {
			this->enqueue(std::bind(func, arg, args...));
		}

		/**
		 * Wraps the given function in a std::future and returns it
		 * while enqueuing the task for execution on this thread pool
		 */
		template <typename F, typename T = typename std::invoke_result<F>::type>
		std::future<T> defer(const F& task) {
			auto* raw_promise = new std::promise<T>();
			std::future<T> future = raw_promise->get_future();

			// kill me
			enqueue([task, raw_promise] () {
				std::unique_ptr<std::promise<T>> promise(raw_promise);

				try {
					promise->set_value(task());
				} catch (...) {
					promise->set_exception(std::current_exception());
				}
			});

			return future;
		}

};