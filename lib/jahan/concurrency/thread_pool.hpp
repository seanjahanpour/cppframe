// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_CONCURRENCY_THREAD_POOL_H_
#define JAHAN_CONCURRENCY_THREAD_POOL_H_

#include <iostream>
#include <vector>
#include <future>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "jahan/signal/manager.h"
#include "jahan/file/logger.h"

namespace jahan::concurrency {
	using SigManager = jahan::signal::Manager;

	class ThreadPool {
	public:
		bool sudden_death_on_termination = true; //if set to false, will try to clear queue before shutting down.

		// Constructs a thread pool with the given number of threads.
		explicit ThreadPool(int num_threads) : m_stop(false)
		{
			for (int i = 0; i < num_threads; i++) {
				add_new_thread();
			}

			//add one additional thread for logging in event of signal trigger
			m_signal_log_thread = std::thread(SigManager::thread_loop);
		}

		// Adds a new task to the thread pool.
		template <class F, class... Args>
		bool add_task(F&& f, Args&&... args) 
		{
			if (m_stop) {
				return false;
			}

			{
				std::unique_lock<std::mutex> lock(m_tasks_mutex);
				m_tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
			}

			std::unique_lock<std::mutex> lock(m_var_mutex);
			m_condition.notify_one();
			return true;
		}

		void add_new_thread()
		{
			using namespace jahan::file;
	
			std::unique_lock<std::mutex> thread_lock(m_worker_mutex);

			m_worker.emplace_back([this]() {
				pthread_setname_np(pthread_self(), "frame pool");
				SigManager::setup_handler([](const int){
					Logger::save();
				}, Logger::log);					

				if (setjmp(SigManager::get_jump_buffer()) == 0) {
					while(!m_stop)
					{
						std::function<void()> task;
						{
							std::unique_lock<std::mutex> task_lock(m_tasks_mutex);
							if(   !m_tasks.empty() && (!(m_stop && sudden_death_on_termination))   ) {
								task = std::move(m_tasks.front());
								m_tasks.pop();
							} else if(!m_stop) {
								task_lock.unlock();
								std::unique_lock<std::mutex> var_lock(m_var_mutex);
								m_condition.wait(var_lock);
							}
						}//release the lock for other threads to pull from queue while we execute the task

						if(task) {
							try {
								task();
							} catch(...) {
								
							}
						
						}	
					}
				} else {
					//signal received. Finish this thread.
					remove_thread(std::this_thread::get_id());
				}
			});		
		}

		void remove_thread(const std::thread::id thread_id)
		{
			/*
			auto it = std::find_if(m_worker.begin(), m_worker.end(), 
			[&thread_id](const std::thread &t)
			{
				return t.get_id() == thread_id;
			});

			// Check if the current thread was found in the vector
			if (it != m_worker.end())
			{
				// Current thread was found
				//it->join();
				std::cout << "Removing TID: " << thread_id << std::endl;

				std::unique_lock<std::mutex> thread_lock(m_worker_mutex);
				m_worker.erase(it);
				std::cout << "Made it TID: " << thread_id << std::endl;
			}
			*/
		}

		~ThreadPool() 
		{
			m_stop = true;

			std::unique_lock<std::mutex> lock(m_var_mutex);
			m_condition.notify_all();
			
			for (auto& worker : m_worker) {
				worker.join();
			}

			m_signal_log_thread.join();
		}

	private:
		std::vector<std::jthread> m_worker;
		std::queue<std::function<void()>> m_tasks;
		std::mutex m_tasks_mutex;
		std::mutex m_var_mutex;
		std::mutex m_worker_mutex;
		std::condition_variable m_condition;
		std::thread m_signal_log_thread;
		bool m_stop;
	};
} //namespace jahan::concurrency
#endif //JAHAN_CONCURRENCY_THREAD_POOL_H_