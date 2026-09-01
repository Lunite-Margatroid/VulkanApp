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
	"renderer/sampler/*.cpp"
	"renderer/mesh/*.cpp"
	"util/*.cpp"
	"renderer/pipeline/renderpass/*.cpp"
	"renderer/pipeline/stage/*.cpp"
	"renderer/material/*.cpp"
	"renderer/entity/*.cpp"
)

#set(HEADERS
#    "Engine.h"
#)
# 头文件
file(GLOB PUBLIC_HEADERS
    "core/*.h"
	"logger/*.h"
	"util/*.hpp"
	"util/*.h"
)

file(GLOB PRIVATE_HEADERS
    "renderer/*.h"
	"renderer/swapchain/*.h"
	"renderer/pipeline/*.h"
	"renderer/buffer/*.h"
	"core/image/*.h"
	"renderer/deviceImage/*.h"
	"renderer/sampler/*.h"
	"renderer/mesh/*.h"
	"renderer/mesh/*.hpp"
	"renderer/pipeline/renderpass/*.h"
	"renderer/pipeline/renderpass/*.hpp"
	"renderer/pipeline/stage/*.h"
	"renderer/pipeline/stage/*.hpp"
	"renderer/material/*.h"
	"renderer/material/*.hpp"
	"renderer/entity/*.h"
	"renderer/entity/*.hpp"
)

# 创建静态库
add_library(vkRenderer STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS})
# 链接vulkan
target_link_libraries(vkRenderer PRIVATE Vulkan::Vulkan)

# 链接slang
target_link_libraries(vkRenderer PRIVATE SlangCompiler)
# 链接logger
target_link_libraries(vkRenderer PUBLIC logger)

# 链接OpenImageIO
if(${OpenImageIO_FOUND})
	target_link_libraries(vkRenderer PRIVATE OpenImageIO::OpenImageIO)
else()
	# 使用输入参数
	if(WIN32)
		target_link_libraries(vkRenderer 
			PRIVATE
				debug "${OIIO_INSTALLED_DIR}/lib/OpenImageIO_d.lib"
				optimized "${OIIO_INSTALLED_DIR}/lib/OpenImageIO.lib"
		)
		target_link_libraries(vkRenderer 
			PRIVATE 
				debug "${OIIO_INSTALLED_DIR}/lib/OpenImageIO_Util_d.lib"
				optimized "${OIIO_INSTALLED_DIR}/lib/OpenImageIO_Util.lib"
		)
	elseif(UNIX)
		target_link_libraries(vkRenderer
			PRIVATE 
				"${OIIO_INSTALLED_DIR}/lib/libOpenImageIO.so"
		)
		target_link_libraries(vkRenderer
			PRIVATE 
				"${OIIO_INSTALLED_DIR}/lib/libOpenImageIO_Util.so"
		)
	else()
		message(FATAL_ERROR "Unsupported platform")
	endif()

	target_include_directories(vkRenderer 
		PRIVATE 
			"${OIIO_INSTALLED_DIR}/include"
	)

endif()



# 使用utf-8编码编译源文件 仅windows
if(WIN32)
	target_compile_options(vkRenderer PRIVATE "/utf-8")
endif()

# 设置包含目录
target_include_directories(vkRenderer 
    PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}/core"
		"${CMAKE_CURRENT_SOURCE_DIR}/util"
    PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/swapchain"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/buffer"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/deviceImage"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/pipeline"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/pipeline/renderpass"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/sampler"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/mesh"	
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/pipeline/stage"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/material"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer/entity"
        "${GLM_INSTALLED_DIR}/include"
		"${SLANG_INSTALLED_DIR}/include"
		"${Vulkan_INCLUDE_DIRS}"
		"${OIIO_INSTALLED_DIR}/include"
		"${VMA_INCLUDE_DIR}"

)


# VMA
if(EXISTS "${VMA_INSTALLED_DIR}/include/vma/vk_mem_alloc.h")
	target_include_directories(vkRenderer PRIVATE "${VMA_INSTALLED_DIR}/include/vma")
else()
	target_include_directories(vkRenderer PRIVATE "${VMA_INSTALLED_DIR}/include")
endif()
