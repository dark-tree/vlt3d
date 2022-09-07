
#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t count) : stop(false) {

	logger::info("Created thread pool ", this, " of size: ", count);

	while( count --> 0 ) {
		this->workers.emplace_back( &ThreadPool::run, this );
	}

}

ThreadPool::~ThreadPool() {
	
	{
		std::unique_lock<std::mutex> lock(this->queue_mutex);
		this->stop = true;
	}

	this->condition.notify_all();
	
	for(auto& worker : this->workers) {
		worker.join();
	}

	logger::info("Stopped thread pool ", this);

}

void ThreadPool::enqueue(const Task& task) {

	{
		std::unique_lock<std::mutex> lock(this->queue_mutex);

		// don't allow enqueueing after stopping the pool
		if( this->stop ) throw std::runtime_error("Unable to add task to a stopped pool!");

		if( this->tasks.size() > 64 ) {
			logger::warn("Thread pool ", this, " can't keep up! ", this->tasks.size(), " tasks awaiting execution!");
		}

		this->tasks.emplace(task);
	}

	this->condition.notify_one();

}

void ThreadPool::run() {

	Task task;
	while(true) {

		{
			std::unique_lock<std::mutex> lock(this->queue_mutex);

			this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
			if(this->stop && this->tasks.empty()) return;
					
			task = std::move( this->tasks.front() );
			this->tasks.pop();
		}

		task();

	}

}

size_t ThreadPool::optimal() {
	return std::max((int) std::thread::hardware_concurrency() - 1, 1);
}

ThreadPool Threads::pool;
std::queue<Task> Threads::synced_tasks;

void Threads::execute() {

	while( !Threads::synced_tasks.empty() ) {
		
		Threads::synced_tasks.front()();
		Threads::synced_tasks.pop();

	}
}

