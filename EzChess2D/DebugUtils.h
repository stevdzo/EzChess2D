#ifndef _DEBUG_UTILS_H
#define _DEBUG_UTILS_H

#include <iostream>
#include <string>
#include <sstream>

template <typename T>
inline void Print(const T _Value) {
	std::ostringstream oss;
	oss << _Value;
	std::cout << "[LOG]: " << oss.str() << std::endl;
}

template <typename T>
inline void Print(const std::string& _Str, const T _Value) {
	std::ostringstream oss;
	oss << _Value;
	std::cout << "[" << _Str << "]: " << oss.str() << std::endl;
}
#endif
