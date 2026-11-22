#include "logger.hpp"

#include <filesystem>
#include <random>

namespace LT {
    logger* logger::s_pInstance = nullptr;

    logger::logger()
        : m_funcOnLog(nullptr)
        , m_pUserData(nullptr)
        , m_nLastFlushTime(0)
    {
        InitLogFile();
    }

    logger::~logger()
    {
        UnregisterOnLog();
        if (m_fileLog.is_open())
        {
            m_fileLog.flush();
            m_fileLog.close();
        }
    }

    void logger::InitLogFile()
    {
        // 在当前目录下创建 ./log 目录
        std::error_code ec;
        std::filesystem::create_directories("./log", ec);
        if (ec)
        {
            printf("[warn] logger: create log directory failed. %s\n", ec.message().c_str());
            return;
        }

        // 文件名 year-month-day-hour-min-second_[RandomInteger].log
        std::time_t curTime = std::time(nullptr);
        std::tm* pLocalTime = std::localtime(&curTime);
        char strTime[32];
        std::strftime(strTime, sizeof(strTime), "%Y-%m-%d-%H-%M-%S", pLocalTime);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 999999);
        const int nRandomInteger = dist(gen);

        const std::string strPath = "./log/" + std::string(strTime) + "_" + std::to_string(nRandomInteger) + ".log";

        m_fileLog.open(strPath, std::ios::out | std::ios::app);
        if (!m_fileLog.is_open())
        {
            printf("[warn] logger: open log file failed. %s\n", strPath.c_str());
        }
    }

    logger& logger::GetInstance()
    {
        // 懒加载 该实例跟随进程生命周期
        if (!s_pInstance)
        {
            s_pInstance = new logger();
        }
        return *s_pInstance;
    }

    void logger::RegisterOnLog(const std::function<void(int, std::string_view, void*)>& funcOnLog, void* pUserData)
    {
        m_funcOnLog = funcOnLog;
        m_pUserData = pUserData;
    }

    void logger::UnregisterOnLog()
    {
        m_funcOnLog = nullptr;
        m_pUserData = nullptr;
    }

    void logger::LogTrace(std::string_view strLog)
    {
        GetInstance().Log<LOG_LEVEL_TRACING>(strLog);
    }

    void logger::LogInfo(std::string_view strLog)
    {
        GetInstance().Log<LOG_LEVEL_INFO>(strLog);
    }

    void logger::LogWarning(std::string_view strLog)
    {
        GetInstance().Log<LOG_LEVEL_WARNING>(strLog);
    }

    void logger::LogError(std::string_view strLog)
    {
        GetInstance().Log<LOG_LEVEL_ERROR>(strLog);
    }

} // namespace LT
