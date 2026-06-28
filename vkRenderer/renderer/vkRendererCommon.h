#pragma once 
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <optional>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <array>
#include <unordered_map>
#include <map>

#define NOMINMAX
#include <windows.h>

#include "ExceptionAssert.h"

#define VK_USE_PLATFORM_WIN32_KHR

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#include "vulkan\vulkan.hpp"
#include "vulkan\vulkan_raii.hpp"

#include "glm\glm.hpp"

#define LOG(strLog) do{ \
		std::time_t curTime = std::time(nullptr);\
		char buffer[64];\
		std::tm* local_time = std::localtime(&curTime);\
		std::strftime(buffer, 64, "[%Y-%m-%d %H:%M:%S]", local_time);\
		printf("%s %s ",buffer, (strLog)); \
		}while(false)
#define PRINT_FILE_LINE() printf("[file: %s, line: %d]", __FILE__, __LINE__)

#define LOG_INFO(...) do{ LOG("[info]");  printf(__VA_ARGS__);printf("\n");}while(false)
#define LOG_WARNING(...) do{ LOG("[warn]");  printf(__VA_ARGS__);printf("\n");}while(false)
#define LOG_ERROR(...) do{ LOG("[error]");  printf(__VA_ARGS__);printf("\n");}while(false)
#define LOG_DEBUG(...) do{ LOG("[debug]");  printf(__VA_ARGS__);printf("\n");}while(false)
#define LOG_TRACING(...) do{ LOG("[trace]"); printf(__VA_ARGS__); printf("\n");}while(false)

#define LOG_ERROR_WITH_FILE(...) do { LOG("[error]"); PRINT_FILE_LINE(); printf(__VA_ARGS__);printf("\n");} while(false)

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

inline std::string ReadText(const std::filesystem::path& filePath) {
    std::string strFilePath = filePath.generic_string();
    std::ifstream file(strFilePath);
    if (!file.is_open()) {
        LOG_ERROR("Can not open file %s.", filePath.generic_string().c_str());
    }

    std::string content;
    content.reserve(std::filesystem::file_size(filePath)); // ‘§∑÷≈‰ø’º‰
    content.assign(std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
    return content;
}