// Author: Sean Jahanpour (sean@jahanpour.com)
//
#include <unordered_map>
#include <iomanip>
#include <ctime>
#include <thread>
#include <chrono>
#include <iostream>

#include <pthread.h>

#include "jahan/file/logger.h"

namespace jahan::file
{
	std::string Logger::m_folder;
	std::deque<LogMessage> Logger::m_log_queue;
	std::mutex Logger::m_queue_mutex;
	std::mutex Logger::m_file_mutex;
	int Logger::m_save_frequency_minutes;

	void Logger::setup(const std::string& folder_path, const int frequency_minutes)
	{
		Logger::m_folder = folder_path;
		m_save_frequency_minutes = frequency_minutes;
	}

	void Logger::thread_loop()
	{
		pthread_setname_np(pthread_self(), "frame logger");
		while(1) {
			std::this_thread::sleep_for(std::chrono::minutes(m_save_frequency_minutes));
			save();
		}
	}

	void Logger::log(const std::string& message, const LogLevel level)
	{
		std::string log_level;
		switch(level) {
		case Critical: log_level = "critical"; break;
		case Error: log_level = "error"; break;
		case Warning: log_level = "warning"; break;
		case Info: log_level = "info"; break;
		case Debug: log_level = "debug"; break;
		}

		LogMessage e = { .message = message, .level = log_level, .timestamp = std::chrono::system_clock::now()};
		
		std::lock_guard<std::mutex> lock(m_queue_mutex);
		m_log_queue.push_back(e);
	}

	bool Logger::save()
	{
		std::lock_guard<std::mutex> qlock(m_queue_mutex);
		std::lock_guard<std::mutex> flock(m_file_mutex);

		std::unordered_map<std::string, std::ofstream>f = {};

		bool ret = true;	//assume success as defualt
		while(!m_log_queue.empty() && ret == true) {
			const LogMessage* e = &m_log_queue.front();
			if(f.find(e->level) == f.end()) {
				//open file for this level
				std::ofstream out(m_folder + "/" + e->level, std::ios::app);

				if(out.is_open()) {
					f[e->level] = std::move(out);
				} else {
					ret = false;
					if((std::chrono::system_clock::now() - std::chrono::minutes(1440)) > e->timestamp) {
						//log has not been save for more than 1 day (1440 min). Just drop it.
						m_log_queue.pop_front();
					}
					std::cout << "Unable to save to the log files. Will try again in " << m_save_frequency_minutes << " min." << std::endl;
					continue;
				}
			}

			{
				std::time_t time_t_now = std::chrono::system_clock::to_time_t(e->timestamp);
				std::tm* local_time = std::localtime(&time_t_now);
				f[e->level] << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << " ";
			}
			f[e->level] << e->message << std::endl;
			m_log_queue.pop_front();
		}

		std::cout << std::boolalpha << ret << std::endl;

		return ret;
	}
}