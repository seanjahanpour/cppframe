// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef APP_DB_AUTH_H_
#define APP_DB_AUTH_H_

#include <string>
#include <queue>
#include <future>
#include <optional>

#include "jahan/db/core.hpp"

namespace app::db {
	using namespace jahan::db;

	class Auth
	{
	public:
		std::string static from_first_wss_message(std::string message)
		{
			std::string ret;

			std::deque<std::string> result = jahan::explode("-", message);
			if(result.size() == 4) {
				//1st should be user id
				//2nd should be crypto use for PHP
				//3rd part should be auth token
				//4th element should be user type

				std::string user = result.front();
				result.pop_front();
				result.pop_front(); //don't need crypto part
				std::string token = result.front();
				result.pop_front();
				std::string type = result.front();
				result.pop_front();

				int userID;
				try {
					userID = std::stoi(user);
				} catch(...) {
					return ret;
				}
				
				bool verified = verify_token(userID, token, type);
				if(verified) {
					ret = get_name(userID, type);
					return ret;
				}
			}

			return ret;
		}

		bool static verify_token(const std::string user, const std::string token, const std::string type = "admin")
		{
			try {
				int i = std::stoi(user);
				return verify_token(i, token, type);
			} catch(...) {
				return false;
			}
		}

		bool static verify_token(const int user, const std::string token, const std::string type = "admin")
		{
			Param p_token, p_user, p_type;
			p_user.setInt(user);
			p_token.setString(token);
			p_type.setString(type);
			
			std::queue<Param> params;
			params.push(p_user);
			params.push(p_token);
			params.push(p_type);

			std::optional<std::string> value = Core::get_value("SELECT id AS value FROM usr_sess WHERE usr = ? AND token = ? AND type = ?", params);

			if(value) {
				try{
					std::stoi(value.value());
					return true;
				} catch(...) {
					return false;
				}
			} else {
				return false;
			}
		}

		std::string static get_name(const int user, const std::string type = "admin")
		{
			Param p_user;
			p_user.setInt(user);
			
			std::queue<Param> params;
			params.push(p_user);
			
			if(type == "admin" || type == "agent") {
				std::string ret("");

				std::optional<std::string> value= Core::get_value("SELECT CONCAT(fname, ' ' ,LEFT(lname,1)) FROM " + type + " WHERE id = ?", params);
				if(value) {
					ret = value.value();

					if(type == "agent") {
						ret = std::to_string(user) + ": " + ret;
					}
				}

				return ret;
			}

			return "";
		}
	};
}

#endif //APP_DB_AUTH_H_