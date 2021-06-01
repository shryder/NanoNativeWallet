#include <iostream>
#include "Logger.h"
#include <ctime>    
#include <chrono>

namespace Logger {
	void info (std::string message) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto time_str = std::ctime(&time_t);
		time_str[strlen(time_str) - 1] = '\0';

		std::cout << "[" << time_str << "] " << message << std::endl;
	}
}