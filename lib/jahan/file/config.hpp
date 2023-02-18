// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_FILE_CONFIG_H_
#define JAHAN_FILE_CONFIG_H_

#include <fstream>
#include <string>
#include <unordered_map>

#include <cctype>

namespace jahan::file {

	class Config {
	public:
		static std::unordered_map<std::string, std::string> settings;

		static bool load(std::string config_file_full_path)
		{
			std::ifstream config_file(config_file_full_path);
			if(config_file.is_open()) {
				std::string line;
				while(getline(config_file, line)) {
					//line.erase(std::remove_if(line.begin(), line.end(), isspace)), line.end();
					remove(line, isspace);
					line = line.substr(0, line.find("#"));

					if(line.empty()) {
						continue;
					}

					size_t eq_pos = line.find("=");
					std::string key = line.substr(0, eq_pos);
					std::string value = line.substr(eq_pos + 1);

					settings[key] = value;
				}

				return true;
			} else {
				return false;
			}
		}

		static std::string get(std::string key, std::string default_value = "")
		{
			std::unordered_map<std::string, std::string>::const_iterator position = settings.find(key);
			if(position == settings.end()) {
				return default_value;
			} else {
				return position->second;
			}
		}
	};

	std::unordered_map<std::string, std::string> Config::settings = {};
}
#endif //JAHAN_FILE_CONFIG_H_