// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_DB_PARAM_H_
#define JAHAN_DB_PARAM_H_

#include <string>
namespace jahan::db {
	class Param
	{
	public:
		enum Types{Integer, String, Unset};
		
		void setInt(int val, bool null_value = false) {
			int_val = val;
			is_null = null_value;
			type = Integer;
		}

		void setString(std::string val, bool null_value = false) {
			str_val = val;
			is_null = null_value;
			type = String;
		}

		int getInt() 
		{
			return int_val;
		}

		std::string getString() 
		{
			return str_val;
		}

		bool isNull() 
		{
			return is_null;
		}

		Types getType() 
		{
			return type;
		}

	protected:	
		Types type = Unset;	
		bool is_null = false;
		int int_val = 0;
		std::string str_val;
	};
} //namespace jahan::db
#endif //JAHAN_DB_PARAM_H_