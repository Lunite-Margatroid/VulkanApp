#pragma once 
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

#define LOG(strLog) do{ \
		std::time_t curTime = std::time(nullptr);\
		char buffer[64];\
		std::tm* local_time = std::localtime(&curTime);\
		std::strftime(buffer, 64, "[%Y-%m-%d %H:%M:%S]", local_time);\
		printf("%s %s ",buffer, strLog); \
		}while(false)

#define LOG_INFO(...) do{ LOG("[info]");  printf(__VA_ARGS__);}while(false)
#define LOG_WARNING(...) do{ LOG("[warn]");  printf(__VA_ARGS__);}while(false)
#define LOG_ERROR(...) do{ LOG("[error]");  printf(__VA_ARGS__);}while(false)
#define LOG_DEBUG(...) do{ LOG("[debug]");  printf(__VA_ARGS__);}while(false)