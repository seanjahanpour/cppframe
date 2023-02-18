// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_HELPER_H_
#define JAHAN_HELPER_H_

#include <string>
#include <deque>
#include <algorithm>
#include <iomanip>


namespace jahan {
	std::deque<std::string> explode(const std::string separator, const std::string str, const bool iqnore_blank = true)
	{
		size_t found = 0;
		size_t pos = 0;
		std::deque<std::string> ret;

		do {
			found = str.find(separator, pos);
			ret.emplace_back(str.substr(pos, found-pos));
			if(iqnore_blank && ret.back().empty()) {
				ret.pop_back();
			}
			pos = found + separator.length();
		} while(found != std::string::npos);

		return ret;
	}

	void remove(std::string& set, const std::function<bool(char)>& unary_predicate)
	{
		set.erase(std::remove_if(set.begin(), set.end(), unary_predicate), set.end());
	}

	std::string time_t_to_string(const time_t& time, const std::string format = std::string("%Y-%m-%d %H:%M:%S"))
	{
		std::tm* local_time = std::localtime(&time);
		char str[50];
		strftime( str, 49, format.c_str() , local_time );
		return std::string(str);
	}
}

#endif //JAHAN_HELPER_H_