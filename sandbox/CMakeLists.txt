# sandbox

# 源文件
set(SOURCES
    sandbox.cpp
)

# 创建可执行文件
add_executable(sandbox ${SOURCES})

# 仅在windows下使用传递utf-8编码选项
if(	WIN32)
	target_compile_options(sandbox PRIVATE "/utf-8")
endif()

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
		"${Vulkan_INCLUDE_DIRS}"
)
# SDL3 包含目录
if(WIN32)
	if(EXISTS "${SDL3_INSTALLED_DIR}/include/SDL3")
		target_include_directories(
			sandbox
				PRIVATE
					"${SDL3_INSTALLED_DIR}/include/SDL3"
		)
	elseif(EXISTS "${SDL3_INSTALLED_DIR}/include")
		target_include_directories(
			sandbox
				PRIVATE
					"${SDL3_INSTALLED_DIR}/include"
		)
		
	else()
		message(FATAL_ERROR "Cannot find SLD3 include dir")
	endif()
elseif(UNIX)
	target_include_directories(sandbox 
		PRIVATE 
			"${SDL3_INSTALLED_DIR}/$<IF:$<CONFIG:Debug>,debug/include,include>"
			)

endif()

if(WIN32)
set_target_properties(
    sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
)
else()

endif()