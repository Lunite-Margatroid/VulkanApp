# root

cmake_minimum_required(VERSION 3.15)
project(VulkanApp)

# 检查是否为 Windows 平台
if(WIN32)
    message(STATUS "Building on Windows")
elseif(UNIX)
    message(STATUS "Building on Unix-like platform")
else()
    message(FATAL_ERROR "Unsupported platform")
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
    set(SDK_DEFAULT_INSTALLED_DIR $ENV{VULKAN_SDK})
else()
    message(STATUS "VULKAN_SDK is not defined")
    set(SDK_DEFAULT_INSTALLED_DIR "")
endif()

# 依赖库目录
# SDL3
if(NOT DEFINED SDL3_INSTALLED_DIR)
    set(SDL3_INSTALLED_DIR "${SDK_DEFAULT_INSTALLED_DIR}" CACHE PATH "SDL3 Installed dir")
endif()
# glm
if(NOT DEFINED GLM_INSTALLED_DIR)
    set(GLM_INSTALLED_DIR "${SDK_DEFAULT_INSTALLED_DIR}" CACHE PATH "glm installed dir")
endif()
# OpenImageIO
if(NOT DEFINED OIIO_INSTALLED_DIR)
    set(OIIO_INSTALLED_DIR "C:/Program Files/OpenImageIO" CACHE PATH "OpenImageIO installed dir")
endif()
# VulkanMemoryAllocator
if(NOT DEFINED VMA_INSTALLED_DIR)
    set(VMA_INSTALLED_DIR "${SDK_DEFAULT_INSTALLED_DIR}" CACHE PATH "VulkanMemoryAllcator installed dir")
endif()

if(NOT DEFINED SLANG_INSTALLED_DIR)
    set(SLANG_INSTALLED_DIR "${SDK_DEFAULT_INSTALLED_DIR}" CACHE PATH "Slang installed dir")
endif()
    
message(STATUS "SDL3_INSTALLED_DIR = ${SDL3_INSTALLED_DIR}")
message(STATUS "GLM_INSTALLED_DIR = ${GLM_INSTALLED_DIR}")
message(STATUS "OIIO_INSTALLED_DIR = ${OIIO_INSTALLED_DIR}")
message(STATUS "VMA_INSTALLED_DIR = ${VMA_INSTALLED_DIR}")
message(STATUS "SLANG_INSTALLED_DIR = ${SLANG_INSTALLED_DIR}")

# 检查依赖库
include(cmake/checkvulkan.cmake)


# 添加子目录
add_subdirectory(logger)
add_subdirectory(SlangCompiler)
add_subdirectory(vkRenderer)
add_subdirectory(sandbox)

# 拷贝dll
include(cmake/cpydll.cmake)
# 拷贝着色器文件
include(cmake/cpyslang.cmake)

# 拷贝资源文件
include(cmake/cpyasset.cmake)