#pragma once

#include "logger.hpp"
#include "external.hpp"

using Task = std::function<void()>;

class TaskQueue {

	private:

		std::mutex queue_mutex;
		std::queue<Task> tasks;

		// producer methods
	public:

		void enqueue(const Task& task) {
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace(task);
		}

		template <typename Func, typename Arg, typename... Args>
		void enqueue(Func func, Arg arg, Args... args) {
			enqueue(std::bind(func, arg, args...));
		}

		// consumer methods
	public:

		void execute() {
			std::vector<Task> locals;
			locals.reserve(locals.size());

			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				while (!tasks.empty()) {
					locals.emplace_back(std::move(tasks.front()));
					tasks.pop();
				}
			}

			for (Task& task : locals) {
				task();
			}
		}

};

class TaskPool {

	private:

		bool stop;

		std::vector<std::thread> workers;
		std::mutex queue_mutex;
		std::queue<Task> tasks;
		std::condition_variable condition;

		void run() {

			Task task;
			while (true) {
				{
					std::unique_lock<std::mutex> lock(queue_mutex);

					// wait for task to appear in tasks queue or for the stop sequance to begin
					condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
					if (stop && tasks.empty()) return;

					task = std::move(tasks.front());
					tasks.pop();
				}

				task();
			}

		}

	public:

		static inline size_t optimal() {
			return std::max((int) std::thread::hardware_concurrency() - 1, 1);
		}

		TaskPool(size_t count = TaskPool::optimal())
		: stop(false) {
			logger::info("Created thread pool ", this, " of size: ", count);

			while (count --> 0) {
				workers.emplace_back(&TaskPool::run, this);
			}
		}

		~TaskPool() {
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				stop = true;
			}

			this->condition.notify_all();

			for(auto& worker : this->workers) {
				worker.join();
			}

			logger::info("Stopped thread pool ", this);
		}

		void enqueue(const Task& task) {
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				// don't allow enqueueing after stopping the pool
				if (stop) {
					throw std::runtime_error("Unable to add task to a stopped pool!");
				}

				if (tasks.size() > 64) {
					logger::warn("Thread pool ", this, " can't keep up! ", tasks.size(), " tasks awaiting execution!");
				}

				tasks.emplace(task);
			}

			condition.notify_one();
		}

		template <typename Func, typename Arg, typename... Args>
		void enqueue(Func func, Arg arg, Args... args) {
			this->enqueue(std::bind(func, arg, args...));
		}

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