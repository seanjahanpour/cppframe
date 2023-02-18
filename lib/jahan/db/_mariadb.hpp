// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_DB_MARIADB_H_
#define JAHAN_DB_MARIADB_H_

#include <functional>
#include <thread>
#include <chrono>
#include <deque>
#include <mariadb/conncpp.hpp>

#include "jahan/file/logger.h"
namespace jahan::db {
	using namespace jahan::file;
	
	class MariaDB {
	public:
		MariaDB(std::string databaseurl, std::string user, std::string password, unsigned int minimum_connection_count = 10, std::function<void(std::string message, Logger::LogLevel log_level)> logger = nullptr)
			:m_minimum_connections(minimum_connection_count), m_user(user), m_password(password), m_db_url(databaseurl), m_logger(logger)
		{
			try {
				m_connections.reserve(minimum_connection_count + 100);
				for(unsigned int i=0; i<minimum_connection_count; i++) {
					m_connections.emplace_back(make_connection());
				}

			} catch(sql::SQLException& e) {
				m_connections.clear();
				log(e.what(), Logger::Critical);
			} catch(...) {
				m_connections.clear();
				log("Unknown error when connecting to MariaDB server.", Logger::Critical);
			}
		}

		std::unique_ptr<sql::Connection> get_connection()
		{
			if(m_connections.size() == 0) {
				try{
					return make_connection();
				} catch(...) {
					log("Program trying to access non-existing MariaDB connection. Unable to create on the fly.");
					return nullptr;
				}
			}

			//we have established connections to provide
			try {
				std::lock_guard<std::mutex> lock(m_connections_mutex);
				//std::unique_ptr<sql::Connection> conn = std::move(m_connections.front());
				std::unique_ptr<sql::Connection> conn = std::move(m_connections.back());
				//m_connections.pop_front();
				m_connections.pop_back();
				return conn;
			} catch (...) {
				log("Unknown error occured when trying to access non-existing MaridaDB connection.");
			}

			return nullptr;
		}

		void return_connection(std::unique_ptr<sql::Connection> connection)
		{
			try {
				if(connection->isValid()) {
					connection->clearWarnings();
					std::lock_guard<std::mutex> lock(m_connections_mutex);
					m_connections.push_back(std::move(connection));
				} else {
					log("Returned connection to the pool is not valid.");
				}
			} catch (...) {
				log("Unable to return connection to connection pool.");
			}
		}

		void watchdog_loop()
		{
			//close extra connections (anything more than the minimum)
			pthread_setname_np(pthread_self(), "frame db dog");
			while(true) {
				std::this_thread::sleep_for(std::chrono::minutes(m_watchdog_loop_frequency_in_minutes));
				
				if(m_connections.size() > (m_minimum_connections)) {
					unsigned int extra_connections = m_connections.size() - m_minimum_connections;
					try{
						std::lock_guard<std::mutex> lock(m_connections_mutex);
						for(unsigned int i =0; i < extra_connections; i++) {
							m_connections.pop_back();
						}
					} catch(sql::SQLSyntaxErrorException& e) {
						log(e.what());
					}
				}
			}
		}

		~MariaDB()
		{
			for(auto& connection : m_connections) {
				connection->close();
			}
		}

	protected:
		unsigned int m_watchdog_loop_frequency_in_minutes = 1;
		unsigned int m_minimum_connections;
		unsigned int m_max_connections = 150;
		std::string m_user;
		std::string m_password;
		sql::Properties m_conn_properties;
		std::string m_db_url;
		//std::deque<std::unique_ptr<sql::Connection>> m_connections;
		//switching to vector that might be better in production. This way is easier to see unused connection over time, and the same connection get reused over and over has better chance of staying alive, and fewer needs for reconnection on verify.
		std::vector<std::unique_ptr<sql::Connection>> m_connections;
		std::function<void(std::string message, Logger::LogLevel log_level)> m_logger;
		std::mutex m_connections_mutex;

		inline void log(std::string message, Logger::LogLevel level = Logger::Error)
		{
			if(m_logger) {
				m_logger(message, level);
			}
		}

		inline std::unique_ptr<sql::Connection> make_connection()
		{
			sql::Driver* driver = sql::mariadb::get_driver_instance();
			sql::Properties conn_properties({
				{"user", m_user},
				{"password", m_password},
				{"autoReconnect", "true"},
				{"tcpRcvBuf", "0x100000"},	//tcpSndBuff also changes this value, putting here to make it obvious
				{"tcpSndBuf", "0x100000"},	//tcpRcvBuff also changes this value, putting here to make it obvious
				{"useCompression", "true"},
				{"socketTimeout", "100000"},
			});
			std::unique_ptr<sql::Connection> c(driver->connect(m_db_url, conn_properties));
			return c;
		}

	};

} //namespace jahan::mariadb
#endif //JAHAN_DB_MARIADB_H_