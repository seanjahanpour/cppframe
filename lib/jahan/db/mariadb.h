// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_DB_MARIADB_H_
#define JAHAN_DB_MARIADB_H_

#include <functional>
#include <deque>

#include <mariadb/conncpp.hpp>

#include "jahan/file/logger.h"

namespace jahan::db {
	using namespace jahan::file;
	
	class MariaDB {
	public:
		MariaDB(std::string databaseurl, std::string user, std::string password, unsigned int init_connection_count = 1, std::function<void(std::string message, Logger::LogLevel log_level)> logger = NULL);

		std::unique_ptr<sql::Connection> get_connection();

		void return_connection(std::unique_ptr<sql::Connection> connection);

		void cleanup_loop();

		~MariaDB();		

	protected:
		unsigned int m_watchdog_loop_frequency_in_minutes;
		unsigned int m_init_connection_count;
		std::string m_user;
		std::string m_password;
		sql::Properties m_conn_properties;
		std::string m_db_url;
		std::deque<std::unique_ptr<sql::Connection>> m_connections;
		std::function<void(std::string message, Logger::LogLevel log_level)> m_logger;
		std::mutex m_connections_mutex;

		void log(std::string message, Logger::LogLevel level = Logger::Error);

		std::unique_ptr<sql::Connection> make_connection();
	};

} //namespace jahan::mariadb
#endif //JAHAN_DB_MARIADB_H_