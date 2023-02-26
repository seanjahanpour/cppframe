// Author: Sean Jahanpour (sean@jahanpour.com), special thanks to Aravind A for help with volatile static sig_atomic_t
//
// Copyright (c) Sean Jahanpour (sean@jahanpour.com), Jahan LLC.  All rights reserved.
#ifndef JAHAN_SIGNAL_MANAGER_H_
#define JAHAN_SIGNAL_MANAGER_H_

#include <functional>
#include <string>

#include <sys/signal.h>
#include <csetjmp>

#include "jahan/file/logger.h"
namespace jahan::signal
{
	using namespace jahan;
	using namespace jahan::file;

	class Manager
	{
	public:
		static void setup_handler(std::function<void(int signal)> call_back = NULL, std::function<void(std::string message, Logger::LogLevel log_level)> logger = NULL);

		static void thread_loop();

	private:
		volatile static sig_atomic_t m_run;
		volatile static sig_atomic_t m_sig_caught;
		static std::function<void(std::string message, Logger::LogLevel log_level)> logger;
		static std::function<void(int signal)> user_notify_callback;

		static void handle_signal();
	};
}


#endif //JAHAN_SIGNAL_MANAGER_H_