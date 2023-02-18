// Author: Sean Jahanpour (sean@jahanpour.com)

#ifndef JAHAN_DEV_BENCHMARKTIMER_H_
#define JAHAN_DEV_BENCHMARKTIMER_H_

#include <iostream>
#include <chrono>
#include <string>

namespace jahan::dev
{
	class BenchmarkTimer
	{
	public:
		BenchmarkTimer()
		{
			start();
		}

		~BenchmarkTimer()
		{
			stop(true);
		}

		void start()
		{
			start_time_ = std::chrono::high_resolution_clock::now();
		}

		void stop(bool print_time = false)
		{
			stop_time_ = std::chrono::high_resolution_clock::now();

			if(print_time) {
				std::cout << get_formatted_time() << std::endl;
			}
		}

		double get_time()
		{
			std::chrono::duration<double> diff = stop_time_ - start_time_;
			double ddiff = diff.count();
			return ddiff;	
		}
 
		std::string get_formatted_time()
		{
			double ddiff = get_time();
			int output;

			std::string unit;

			if(ddiff > 9)	{ //larger than 9S
				unit = " S";	//only show seconds
				output = ddiff;
			} else if(ddiff > 0.9) { //larger than 9mS
				unit = " mS";	//only show miliseconds
				output = (ddiff * 1000);
			} else if(ddiff > 9.0e-6) { //larger than 9uS
				unit = " uS";
				output = (ddiff * 1.0e6);
			} else {
				unit = " nS";
				output = (ddiff * 1.0e9);
			}

			return std::to_string(output) + unit;		
		}
	private:
		std::chrono::time_point< std::chrono::high_resolution_clock> start_time_;
		std::chrono::time_point< std::chrono::high_resolution_clock> stop_time_;
	};
}
#endif //LIB_BENCHMARKTIMER_H_