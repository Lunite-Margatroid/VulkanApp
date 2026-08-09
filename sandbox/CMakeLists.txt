# sandbox

# 源文件
set(SOURCES
    sandbox.cpp
)

# 创建可执行文件
add_executable(sandbox ${SOURCES})

# 链接静态库
if(WIN32)
    target_link_libraries(sandbox 
        PRIVATE 
            vkRenderer 
            debug "${SDL3_INSTALLED_DIR}/lib/SDL3d.lib"
            optimized "${SDL3_INSTALLED_DIR}/lib/SDL3.lib"
    )
elseif(UNIX)
    target_link_libraries(sandbox
        PRIVATE
            vkRenderer
            debug "${SDL3_INSTALLED_DIR}/debug/lib/libSDL3.so"
            optimized "${SDL3_INSTALLED_DIR}/lib/libSDL3.so"
        )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()


# 设置包含目录
target_include_directories(sandbox 
    PRIVATE 
        "${CMAKE_SOURCE_DIR}/vkRenderer/core"
        "${CMAKE_SOURCE_DIR}/vkRenderer/logger"
        "${SDL3_INSTALLED_DIR}/$<IF:$<CONFIG:Debug>,debug/include,include>"
		"${Vulkan_INCLUDE_DIRS}"
)

set_target_properties(
    sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"

)