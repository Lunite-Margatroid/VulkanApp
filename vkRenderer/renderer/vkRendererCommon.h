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

#if defined _WIN32

#define NOMINMAX
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#else

using BYTE = uint8_t;


#endif

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "logger.hpp"

constexpr int64_t INVALID_ITEM_ID = -1;
