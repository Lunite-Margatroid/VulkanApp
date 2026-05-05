#pragma once 
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <optional>
#include <memory>
#include <algorithm>

#define NOMINMAX
#include <windows.h>

#define LOG(strLog) do{ \
		std::time_t curTime = std::time(nullptr);\
		char buffer[64];\
		std::tm* local_time = std::localtime(&curTime);\
		std::strftime(buffer, 64, "[%Y-%m-%d %H:%M:%S]", local_time);\
		printf("%s %s ",buffer, strLog); \
		}while(false)
#define PRINT_FILE_LINE() printf("[file: %s, line: %d]", __FILE__, __LINE__)

#define LOG_INFO(...) do{ LOG("[info]");  printf(__VA_ARGS__);}while(false)
#define LOG_WARNING(...) do{ LOG("[warn]");  printf(__VA_ARGS__);}while(false)
#define LOG_ERROR(...) do{ LOG("[error]");  printf(__VA_ARGS__);}while(false)
#define LOG_DEBUG(...) do{ LOG("[debug]");  printf(__VA_ARGS__);}while(false)
#define LOG_TRACING(...) do{ LOG("[trace]"); printf(__VA_ARGS__); }while(false)

#define LOG_ERROR_WITH_FILE(...) do { LOG("[error]"); PRINT_FILE_LINE() printf(__VA_ARGS__);} while(false)