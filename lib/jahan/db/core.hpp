// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_DB_CORE_H_
#define JAHAN_DB_CORE_H_

#include <iostream>
#include <functional>
#include <memory>
#include <queue>
#include <optional>

#include <mariadb/conncpp.hpp>

#include "jahan/file/logger.h"
#include "jahan/db/mariadb.h"
#include "jahan/db/param.hpp"

namespace jahan::db {
	using namespace jahan::file;
	class Core
	{
	public:
		void static set_db(std::unique_ptr<MariaDB> connector)
		{
			m_db = std::move(connector);
		}

		void static set_logger(std::function<void(const std::string message, Logger::LogLevel log_level)> logger)
		{
			m_logger = logger;
		}

		void static set_param(std::shared_ptr<sql::PreparedStatement> stmt,const int32_t index, Param param)
		{
			if(!stmt) {
				return;
			}

			switch(param.getType()) {
			case Param::Integer:
				if(param.isNull()) {
					stmt->setNull(index, 3); //https://github.com/mariadb-corporation/mariadb-connector-cpp/blob/master/src/ColumnType.cpp
				} else{
					stmt->setInt(index, param.getInt());
				}
				break;
			case Param::String:
				if(param.isNull()) {
					stmt->setNull(index, 254); //https://github.com/mariadb-corporation/mariadb-connector-cpp/blob/master/src/ColumnType.cpp
				} else{
					stmt->setString(index, param.getString());
				}
				break;
			case Param::Unset:
				break;
			}
		}

		std::optional<std::string> static get_value(const std::string query)
		{
			std::queue<Param> q;
			return get_value(query, q);
		}
		std::optional<std::string> static get_value(const std::string query, std::queue<Param>& params)
		{
			std::string ret("");
			try{
				std::unique_ptr<sql::Connection> connection = m_db->get_connection();
				if(connection) {
					{
						std::shared_ptr<sql::PreparedStatement> stmt(connection->prepareStatement(query));

						int32_t i = 1;
						while (!params.empty()) {
							auto param = params.front();
							params.pop();

							set_param(stmt, i, param);
							i++;
						}
						
						sql::ResultSet* res = stmt->executeQuery();
						if(res->rowsCount() > 0) {
							res->next();
							ret = res->getString(1);							
						}

						res->~ResultSet();
					}
					m_db->return_connection(std::move(connection));

					return ret;
				}
			} catch(sql::SQLException& e) {
				log(e.what());
			} catch(...) {
				log("We are dying here. core.hpp", Logger::Critical);
			}

			return {};
		}

		void static db_dog();

	protected:
		static std::unique_ptr<MariaDB> m_db;
		static std::function<void(const std::string message, const Logger::LogLevel log_level)> m_logger;
		
		inline void static log(const std::string message, const Logger::LogLevel level = Logger::Error)
		{
			if(m_logger) {
				m_logger(message, level);
			}
		}
	};

	std::unique_ptr<MariaDB> Core::m_db;
	std::function<void(const std::string message, const Logger::LogLevel log_level)> Core::m_logger;

	void Core::db_dog()
	{
		m_db->cleanup_loop();
	}

}//end namespace jahan::db
#endif //JAHAN_DB_CORE_H_