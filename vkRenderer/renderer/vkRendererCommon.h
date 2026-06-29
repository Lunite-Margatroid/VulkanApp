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

#include "logger.h"

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