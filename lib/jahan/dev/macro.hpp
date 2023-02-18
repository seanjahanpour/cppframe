// Author: Sean Jahanpour (sean@jahanpour.com)

#ifndef JAHAN_DEV_MACRO_H_
#define JAHAN_DEV_MACRO_H_

#define JAHAN_DEBUG(...) std::cout << "Debug from: " << __FILE__ << " Line: " << __LINE__ << #__VA_ARGS__ << ";\n"; __VA_ARGS__

#endif //LIB_DEV_MACRO_H_