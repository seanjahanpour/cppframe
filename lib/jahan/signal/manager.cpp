//Author: Sean Jahanpour (sean@jahanpour.com) & Aravind A.
//
#include <iostream>
#include <thread>

#include "jahan/signal/manager.h"
namespace jahan::signal {
	std::function<void(std::string message, Logger::LogLevel log_level)> Manager::logger;
	std::function<void(int signal)> Manager::user_notify_callback;
	volatile sig_atomic_t Manager::m_run;
	volatile sig_atomic_t Manager::m_sig_caught;
	thread_local std::jmp_buf Manager::m_jump_buffer;

	void Manager::setup_handler(std::function<void(int signal)> call_back,
		std::function<void(std::string message, Logger::LogLevel log_level)> logger)
	{
		Manager::logger = logger;
		user_notify_callback = call_back;
		m_run = 1;
		m_sig_caught = 0;

		struct sigaction sa_act;
		sa_act.sa_handler = [](int signum){
			m_run = 0;
			m_sig_caught = signum;
			longjmp(get_jump_buffer(), 1);
		};

		//sigemptyset(&sa_act.sa_mask);
		sigfillset(&sa_act.sa_mask);
		sa_act.sa_flags = 0;

		//fatal signals
		sigaction(SIGBUS,&sa_act,nullptr);
		sigaction(SIGFPE,&sa_act,nullptr);
		sigaction(SIGHUP,&sa_act,nullptr);
		sigaction(SIGILL,&sa_act,nullptr);
		sigaction(SIGQUIT,&sa_act,nullptr);
		sigaction(SIGSEGV,&sa_act,nullptr);
		sigaction(SIGTERM,&sa_act,nullptr);
		//sigaction(SIGKILL,&sa_act,nullptr);
		
		//warning signals
		sigaction(SIGALRM,&sa_act,nullptr);
		sigaction(SIGVTALRM,&sa_act,nullptr);
		sigaction(SIGPROF,&sa_act,nullptr);
		sigaction(SIGXCPU,&sa_act,nullptr);
	}

	void Manager::thread_loop()
	{
		pthread_setname_np(pthread_self(), "frame signal");
		while(true) {
			while(m_run) {
				sleep(1);
			}

			handle_signal();
		}
	}

	void Manager::handle_signal()
	{
		std::string message("Signal:");
		message.append(std::to_string(m_sig_caught));
		Logger::LogLevel log_level = Logger::Error;

		switch (m_sig_caught)
		{
		case SIGALRM:		//alarm, sent when time limit set in preceding alarm setting such as setitimer
		case SIGVTALRM:
		case SIGPROF:
		case SIGXCPU:		//excess CPU usage defined by user

		case SIGBUS:		//e.g. incorrect memory access
		case SIGFPE:		//e.g. division by zero
		case SIGHUP:		//e.g. terminal closed
		case SIGILL:		//illegal, unknown, malformed, or privileged instruction
		case SIGQUIT:		//user request quit
		case SIGSEGV:		//signal segmentation violation (invalid virtual memory reference)
		case SIGTERM:		//terminate
				message += " Terminating";
				log_level = Logger::Critical;
			break;
		}

		if(logger) {
			logger(message, log_level);
		} else {
			std::cerr << message << std::endl;
		}
		
		if(user_notify_callback) {
			user_notify_callback(m_sig_caught);
		}

		
		m_sig_caught = 0;
		m_run = 1;
	}

	std::jmp_buf& Manager::get_jump_buffer()
	{
		return m_jump_buffer;
	}
} //namespace jahan::signal