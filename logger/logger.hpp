#pragma once

#include <string>
#include <exception>
#include "ExceptionAssert.hpp"

#include <functional>
#include <string_view>
#include <cstdio>
#include <ctime>
#include <fstream>

#define LOG_LEVEL_TRACING       4
#define LOG_LEVEL_INFO          5
#define LOG_LEVEL_WARNING       6
#define LOG_LEVEL_ERROR         7

// 日志级别字符串
#define STR_LOG_LEVEL_TRACING   "trace"
#define STR_LOG_LEVEL_INFO      "info"
#define STR_LOG_LEVEL_WARNING   "warn"
#define STR_LOG_LEVEL_ERROR     "error"

#define LOG_LEVEL               LOG_LEVEL_INFO

#define LOG(strLog) do{ \
		std::time_t curTime = std::time(nullptr);\
		char buffer[64];\
		std::tm* local_time = std::localtime(&curTime);\
		std::strftime(buffer, 64, "[%Y-%m-%d %H:%M:%S]", local_time);\
		printf("%s %s ",buffer, (strLog)); \
		}while(false)
#define PRINT_FILE_LINE() printf("[file: %s, line: %d]", __FILE__, __LINE__)



// 日志宏 格式化为字符串后通过class logger转发
// 已注册回调时交给回调 未注册时由logger输出到标准输出
// 用法与printf一致
#define LT_LOG_IMPL(strLoggerFunc, ...) do{ \
		char strLogBuf[256];\
		snprintf(strLogBuf, sizeof(strLogBuf), __VA_ARGS__);\
		LT::logger::strLoggerFunc(strLogBuf);\
	}while(false)

#if LOG_LEVEL <= LOG_LEVEL_TRACING
#define LOG_TRACING(...) LT_LOG_IMPL(LogTrace, __VA_ARGS__)
#else
#define LOG_TRACING(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(...) LT_LOG_IMPL(LogInfo, __VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(...) LT_LOG_IMPL(LogWarning, __VA_ARGS__)
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(...) LT_LOG_IMPL(LogError, __VA_ARGS__)
// 附带文件名和行号
#define LOG_ERROR_WITH_FILE(...) do{ \
		char strLogBuf[512];\
		int nPrefixLen = snprintf(strLogBuf, sizeof(strLogBuf), "[file: %s, line: %d] ", __FILE__, __LINE__);\
		if (nPrefixLen > 0 && static_cast<size_t>(nPrefixLen) < sizeof(strLogBuf))\
		{\
			snprintf(strLogBuf + nPrefixLen, sizeof(strLogBuf) - static_cast<size_t>(nPrefixLen), __VA_ARGS__);\
		}\
		LT::logger::LogError(strLogBuf);\
	}while(false)
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

namespace LT {
    // 日志单例
    // 将日志以回调的形式转发给已注册的接收者(例如渲染到屏幕上的日志)
    // 未注册回调时输出到标准输出
    // 日志额外写入 ./log/ 下的日志文件
    class logger {
    private:
        std::function<void(int, std::string_view, void*)> m_funcOnLog;
        void* m_pUserData;

        // 日志文件
        std::ofstream m_fileLog;
        // 上一次flush的时间(秒)
        std::time_t m_nLastFlushTime;

        logger();
        ~logger();

        // 创建日志文件 ./log/year-month-day-hour-min-second_[RandomInteger].log
        void InitLogFile();

        // 五法则 禁止拷贝和移动
        logger& operator = (const logger&) = delete;
        logger& operator = (logger&&) = delete;

        logger(const logger&) = delete;
        logger(logger&&) = delete;

        template<int log_level>
        void Log(std::string_view strLog) {
            // 日志级别转字符串
            const char* pStrLevel = "unknown";
            if constexpr (log_level == LOG_LEVEL_TRACING)
            {
                pStrLevel = STR_LOG_LEVEL_TRACING;
            }
            else if constexpr (log_level == LOG_LEVEL_INFO)
            {
                pStrLevel = STR_LOG_LEVEL_INFO;
            }
            else if constexpr (log_level == LOG_LEVEL_WARNING)
            {
                pStrLevel = STR_LOG_LEVEL_WARNING;
            }
            else if constexpr (log_level == LOG_LEVEL_ERROR)
            {
                pStrLevel = STR_LOG_LEVEL_ERROR;
            }

            // 组装一行日志 [log_level][hour:min:second] ...
            std::time_t curTime = std::time(nullptr);
            std::tm* pLocalTime = std::localtime(&curTime);
            char strTime[16];
            snprintf(strTime, sizeof(strTime), "%02d:%02d:%02d",
                pLocalTime->tm_hour,
                pLocalTime->tm_min,
                pLocalTime->tm_sec
            );

            std::string strLine;
            strLine.reserve(strLog.size() + 32);
            strLine += '[';
            strLine += pStrLevel;
            strLine += "][";
            strLine += strTime;
            strLine += "] ";
            strLine.append(strLog);

            // 额外输出到文件
            if (m_fileLog.is_open())
            {
                m_fileLog << strLine << '\n';

                // 降低flush频率 每秒最多flush一次
                // 错误日志立即flush
                bool bFlush = (curTime - m_nLastFlushTime >= 1);
                if constexpr (log_level == LOG_LEVEL_ERROR)
                {
                    bFlush = true;
                }

                if (bFlush)
                {
                    m_fileLog.flush();
                    m_nLastFlushTime = curTime;
                }
            }

            if (m_funcOnLog)
            {
                m_funcOnLog(log_level, strLog, m_pUserData);
                return;
            }

            // 未注册回调 输出到标准输出
            printf("%s\n", strLine.c_str());
        }

        static logger* s_pInstance;

    public:
        static logger& GetInstance();

        // 注册日志回调 pUserData 原样传回 不转移所有权
        // 同一时间只保留一个回调 重复注册会覆盖
        void RegisterOnLog(const std::function<void(int, std::string_view, void*)>& funcOnLog, void* pUserData);
        void UnregisterOnLog();

        // strLog 仅在回调调用期间有效 需要保留请自行拷贝
        static void LogTrace(std::string_view strLog);
        static void LogInfo(std::string_view strLog);
        static void LogWarning(std::string_view strLog);
        static void LogError(std::string_view strLog);
    };

} // namespace LT