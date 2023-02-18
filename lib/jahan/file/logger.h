// Author: Sean Jahanpour (sean@jahanpour.com)
//
#include <fstream>
#include <string>
#include <deque>
#include <mutex>
#include <chrono>

#ifndef JAHAN_FILE_LOGGER_H_
#define JAHAN_FILE_LOGGER_H_
namespace jahan::file
{
	struct LogMessage {
		std::string message;
		std::string level;
		std::chrono::system_clock::time_point timestamp;
	};

	class Logger
	{
	public:
		enum LogLevel {Critical, Error, Warning, Info, Debug};
		static void setup(const std::string& folder_path, const int frequency_minutes);
		static void thread_loop();
		static void log(const std::string& message, const LogLevel level);
		static bool save();
	protected:
		static std::string m_folder;
		static std::deque<LogMessage> m_log_queue;
		static std::mutex m_queue_mutex;
		static std::mutex m_file_mutex;
		static int m_save_frequency_minutes;
	};


}// end namespace jahan::file
#endif //JAHAN_FILE_LOGGER_H_