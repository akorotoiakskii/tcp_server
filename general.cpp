#include "general.h"







void ThreadPool::stop() {
	pool_terminated = true;
	join();
}

void ThreadPool::start(uint thread_count = std::thread::hardware_concurrency()) {
	if(!pool_terminated) return;
	pool_terminated = false;
	setupThreadPool(thread_count);
}


//template<typename F, typename... Arg>
//void ThreadPool::addJob(const F& job, const Arg&... args) {addJob([job, args...]{job(args...);});}



uint ThreadPool::getThreadCount() const {return thread_pool.size();}



