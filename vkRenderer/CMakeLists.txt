# vkRenderer

# 源文件
file(GLOB SOURCES 
    "renderer/*.cpp"
	"renderer/swapchain/*.cpp"
	"renderer/pipeline/*.cpp"
    "exception/*.cpp"
	"core/*.cpp"
	"core/image/*.cpp"
	"renderer/buffer/*.cpp"
	"logger/*.cpp"
	"renderer/deviceImage/*.cpp"
)

#set(HEADERS
#    "Engine.h"
#)
# 头文件
file(GLOB PUBLIC_HEADERS
    "core/*.h"
	"logger/*.h"
)

file(GLOB PRIVATE_HEADERS
    "renderer/*.h"
	"renderer/swapchain/*.h"
	"renderer/pipeline/*.h"
    "exception/*.h"
	"renderer/buffer/*.h"
	"core/image/*.h"
	"renderer/deviceImage/*.h"
)

# 创建静态库
add_library(vkRenderer STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS})
# 链接vulkan
target_link_libraries(vkRenderer PRIVATE Vulkan::Vulkan)
# 链接slang
target_link_libraries(vkRenderer PRIVATE "$ENV{VK_SDK_PATH}/Lib/slang.lib")
# 谅解OpenImageIO
target_link_libraries(vkRenderer PRIVATE "${OIIO_INSTALLED_DIR}/lib/OpenImageIO.lib")


target_compile_options(vkRenderer PRIVATE "/utf-8")

# 设置包含目录
target_include_directories(vkRenderer 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core"
    PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/swapchain"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer"
        "${CMAKE_CURRENT_SOURCE_DIR}/exception"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/buffer"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/pipeline"
		"${CMAKE_CURRENT_SOURCE_DIR}/logger"
        "${GLM_INSTALLED_DIR}/Include"
		"${OIIO_INSTALLED_DIR}/include"

)