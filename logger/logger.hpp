#pragma once

#include <string>
#include <exception>
#include "ExceptionAssert.hpp"

#define LOG_LEVEL_TRACING       4
#define LOG_LEVEL_INFO          5
#define LOG_LEVEL_WARNING       6
#define LOG_LEVEL_ERROR         7

#define LOG_LEVEL               LOG_LEVEL_INFO

#define LOG(strLog) do{ \
		std::time_t curTime = std::time(nullptr);\
		char buffer[64];\
		std::tm* local_time = std::localtime(&curTime);\
		std::strftime(buffer, 64, "[%Y-%m-%d %H:%M:%S]", local_time);\
		printf("%s %s ",buffer, (strLog)); \
		}while(false)
#define PRINT_FILE_LINE() printf("[file: %s, line: %d]", __FILE__, __LINE__)



#if LOG_LEVEL <= LOG_LEVEL_TRACING
#define LOG_TRACING(...) do{ LOG("[trace]"); printf(__VA_ARGS__); printf("\n");}while(false)
#else
#define LOG_TRACING(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(...) do{ LOG("[info]");  printf(__VA_ARGS__);printf("\n");}while(false)
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(...) do{ LOG("[warn]");  printf(__VA_ARGS__);printf("\n");}while(false)
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(...) do{ LOG("[error]");  printf(__VA_ARGS__);printf("\n");}while(false)
#define LOG_ERROR_WITH_FILE(...) do { LOG("[error]"); PRINT_FILE_LINE(); printf(__VA_ARGS__);printf("\n");} while(false)
#else
#define LOG_ERROR(...)
#define LOG_ERROR_WITH_FILE(...)
#endif


#define RENDERER_ASSERT(mr_bAssert, ...) do{ \
    if(!(mr_bAssert)) \
        {\
            LOG("[ASSERT_FAILED]"); \
            PRINT_FILE_LINE();\
            char msgBuf[192];\
            sprintf(msgBuf, __VA_ARGS__);\
            printf(msgBuf);\
            printf("\n");\
            throw LT::ExceptionAssert(msgBuf);\
    }}\
while(false)