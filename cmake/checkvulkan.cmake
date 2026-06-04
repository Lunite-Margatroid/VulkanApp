# CheckVulkan.cmake
# 寻找Vulkan, Slang
find_package(Vulkan REQUIRED)

if(${Vulkan_FOUND})
	message(STATUS "Vulkan found")
	message(STATUS "vulkan include dir: ${Vulkan_INCLUDE_DIRS}")
	message(STATUS "vulkan library: ${Vulkan_LIBRARIES}")
else()
	message(FATAL_ERROR "vulkan not found")
endif()

#find_package(Slang REQUIRED)
#
#if(${Slang_FOUND})
#	message(STATUS "Slang found")
#	message(STATUS "Slang include dir: ${Slang_INCLUDE_DIRS}")
#	message(STATUS "Slang library: ${Slang_LIBRARIES}")
#else()
#	message(FATAL_ERROR "Slang not found")
#endif()