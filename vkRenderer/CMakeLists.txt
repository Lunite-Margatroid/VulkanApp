# vkRenderer

# 源文件
file(GLOB SOURCES 
    "src/*.cpp"
	"SwapChain/*.cpp"
    "exception/*.cpp"
	"core/*.cpp"
	"core/buffer/*.cpp"
)

set(HEADERS
    vkRenderer.h
)
# 头文件
file(GLOB PUBLIC_HEADERS
    "include/*.h"
)

file(GLOB PRIVATE_HEADERS
    "header/*.h"
	"SwapChain/*.h"
    "exception/*.h"
	"core/*.h"
	"core/buffer/*.h"
)

# 创建静态库
add_library(vkRenderer STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS})
# 链接vulkan
target_link_libraries(vkRenderer PRIVATE Vulkan::Vulkan)
# 链接slang
target_link_libraries(vkRenderer PRIVATE "$ENV{VK_SDK_PATH}/Lib/slang.lib")

# 设置包含目录
target_include_directories(vkRenderer 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/header"
		"${CMAKE_CURRENT_SOURCE_DIR}/SwapChain"
        "${CMAKE_CURRENT_SOURCE_DIR}/exception"
		"${CMAKE_CURRENT_SOURCE_DIR}/core"
		"${CMAKE_CURRENT_SOURCE_DIR}/core/buffer"
		
        "${GLM_INSTALLED_DIR}/Include"
)