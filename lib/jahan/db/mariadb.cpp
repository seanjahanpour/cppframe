// Author: Sean Jahanpour (sean@jahanpour.com)
//
#include <thread>
#include <chrono>

#include "jahan/db/mariadb.h"

namespace jahan::db {
	MariaDB::MariaDB(std::string databaseurl, std::string user, std::string password, unsigned int init_connection_count, std::function<void(std::string message, Logger::LogLevel log_level)> logger)
	 	:m_watchdog_loop_frequency_in_minutes(5),
		 m_init_connection_count(init_connection_count), 
		 m_user(user),
		 m_password(password),
		 m_db_url(databaseurl),
		 m_logger(logger)
	{
		try {
			for(unsigned int i=0; i<init_connection_count; i++) {
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

	MariaDB::~MariaDB()
	{
		for(auto& connection : m_connections) {
			connection->close();
		}
	}

	std::unique_ptr<sql::Connection> MariaDB::get_connection()
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
			std::unique_ptr<sql::Connection> conn = std::move(m_connections.front());
			m_connections.pop_front();
			return conn;
		} catch (...) {
			log("Unknown error occured when trying to access non-existing MaridaDB connection.");
		}

		return nullptr;
	}

	void MariaDB::return_connection(std::unique_ptr<sql::Connection> connection)
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

	void MariaDB::cleanup_loop()
	{
		//close extra connections (anything more than the minimum)
		pthread_setname_np(pthread_self(), "frame db check");
		while(true) {
			std::this_thread::sleep_for(std::chrono::minutes(m_watchdog_loop_frequency_in_minutes));
			
			if(m_connections.size() > (m_init_connection_count)) {
				unsigned int extra_connections = m_connections.size() - m_init_connection_count;
				try {
					std::lock_guard<std::mutex> lock(m_connections_mutex);
					for(unsigned int i =0; i < extra_connections; i++) {
						m_connections.front()->close();
						m_connections.pop_front();
					}
				} catch(sql::SQLSyntaxErrorException& e) {
					log(e.what());
				}
			}
		}
	}

	void MariaDB::log(std::string message, Logger::LogLevel level)
	{
		if(m_logger) {
			m_logger(message, level);
		}
	}

	std::unique_ptr<sql::Connection> MariaDB::make_connection()
	{
		sql::Driver* driver = sql::mariadb::get_driver_instance();
		sql::Properties conn_properties({
			{"user", m_user},
			{"password", m_password},
			{"autoReconnect", "true"},
			{"tcpRcvBuf", "0x100000"},	//tcpSndBuff also changes this value, putting here to make it obvious
			{"tcpSndBuf", "0x100000"},	//tcpRcvBuff also changes this value, putting here to make it obvious
			{"useCompression", "true"},
			{"socketTimeout", "10000"},
		});
		std::unique_ptr<sql::Connection> c(driver->connect(m_db_url, conn_properties));
		return c;
	}

} //namespace jahan::mariadb