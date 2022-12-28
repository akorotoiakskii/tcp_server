#ifndef GENERAL_H
#define GENERAL_H

#include <sys/socket.h>
#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <malloc.h>
#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

// POSIX
typedef int Socket;
typedef int ka_prop_t;


enum class SocketState : uint8_t {
  up = 0,
  err_socket_init = 1,
  err_socket_bind = 2,
  err_scoket_keep_alive = 3,
  err_socket_listening = 4,
  close = 5
};



enum class SocketStatus : uint8_t {
  connected = 0,
  err_socket_init = 1,
  err_socket_bind = 2,
  err_socket_connect = 3,
  disconnected = 4
};

/// Simple thread pool implementation
class ThreadPool {
  std::vector<std::thread> thread_pool;
  std::queue<std::function<void()>> job_queue;
  std::mutex queue_mtx;
  std::condition_variable condition;
  std::atomic<bool> pool_terminated;



  
public:
  void workerLoop() {
    std::function<void()> job;
    while (!pool_terminated) {
      {
        std::unique_lock<std::mutex> lock(queue_mtx);
        condition.wait(lock, [this](){return !job_queue.empty() || pool_terminated;});
        if(pool_terminated) return;
        job = job_queue.front();
        job_queue.pop();
      }
      job();
    }
}

  void setupThreadPool(uint thread_count) {
	thread_pool.clear();
	for(uint i = 0; i < thread_count; ++i)
		thread_pool.emplace_back(&ThreadPool::workerLoop, this);
  }
  void join() {for(auto& thread : thread_pool) thread.join();}
  void detach() {for(auto& thread : thread_pool) thread.detach();}
  uint getThreadCount() const;
  void dropUnstartedJobs() {
    pool_terminated = true;
    join();
    pool_terminated = false;
    // Clear jobs in queue
    std::queue<std::function<void()>> empty;
    std::swap(job_queue, empty);
    // reset thread pool
    setupThreadPool(thread_pool.size());
  }
  void stop();
  void start(uint thread_count);
  ThreadPool() {
    uint thread_count = std::thread::hardware_concurrency();
    setupThreadPool(thread_count);
  }

  ~ThreadPool() {
    pool_terminated = true;
    join();
  }


 // template<typename F, typename... Arg>
  //void addJob(const F& job, const Arg&... args);
  
  template<typename F>
  void addJob(F job) {
    if(pool_terminated) return;
    {
      std::unique_lock<std::mutex> lock(queue_mtx);
      job_queue.push(std::function<void()>(job));
    }
    condition.notify_one();
  }


};

enum class SocketType : uint8_t {
  client_socket = 0,
  server_socket = 1
};


#endif // GENERAL_H
