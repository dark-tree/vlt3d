#pragma once

#include <logger.hpp>
#include <include/external.hpp>

using Task = std::function<void()>;

class ThreadPool {

	private:

		bool stop;
	
		std::vector<std::thread> workers;
		std::mutex queue_mutex;
		std::queue<Task> tasks;
		std::condition_variable condition;

		void run();

	public:

		ThreadPool(size_t count = ThreadPool::optimal());
		~ThreadPool();

		void enqueue(const Task& task);

		template< typename Func, typename Arg, typename... Args >
		void enqueue( Func func, Arg arg, Args... args ) {
			this->enqueue( std::bind(func, arg, args...) );
		}

		static size_t optimal();

};

// I'm not 100% sure if one global Thread Pool is a good idea,
// but for the time being it does the job
class Threads {

	private:

		static ThreadPool pool;
		static std::queue<Task> synced_tasks;

	public:

		template< typename Func, typename... Args >
		static void async(Func func, Args... args) {
			Threads::pool.enqueue(func, args...);
		}

		template< typename Func, typename... Args >
		static void synced(Func func, Args... args) {
			Threads::synced_tasks.emplace( std::bind(func, args...) );
		}

		static void execute();

};

