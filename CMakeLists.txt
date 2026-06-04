# root

cmake_minimum_required(VERSION 3.15)
project(VulkanApp)

# 检查是否为 Windows 平台
if(NOT WIN32)
    message(FATAL_ERROR "This project only supports Windows platform.")
endif()

# c++ 20
set(CMAKE_CXX_STANDARD 20)

# 输出目录
# 静态库
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
# 可执行文件 动态库
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)




if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "VULKAN_SDK = $ENV{VULKAN_SDK}")
    set(ENV_VULKAN_PATH $ENV{VULKAN_SDK})
else()
    message(FATAL_ERROR "VULKAN_SDK is not defined")
    set(ENV_VULKAN_PATH "")
endif()

# 依赖库目录
# SDL2
set(SDL2_INSTALLED_DIR "${ENV_VULKAN_PATH}" CACHE PATH "SDL2 Installed dir")
# glm
set(GLM_INSTALLED_DIR "${ENV_VULKAN_PATH}" CACHE PATH "glm installed dir")

message(STATUS "SDL2_INSTALLED_DIR = ${SDL2_INSTALLED_DIR}")
message(STATUS "GLM_INSTALLED_DIR = ${GLM_INSTALLED_DIR}")

# 检查vulkan Slang
include(cmake/checkvulkan.cmake)


# 添加子目录
add_subdirectory(vkRenderer)
add_subdirectory(sandbox)

# 拷贝dll
include(cmake/cpydll.cmake)
# 拷贝着色器文件
include(cmake/cpyslang.cmake)