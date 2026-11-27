# sandbox

# 源文件
file(GLOB SOURCES
    "./*.cpp"
)

file(GLOB HEADERS
	"./*.hpp"
	"./*.h"
)


# 创建可执行文件
add_executable(sandbox ${SOURCES} ${HEADERS})

# 仅在windows下使用传递utf-8编码选项
if(	WIN32)
	target_compile_options(sandbox PRIVATE "/utf-8")
endif()

# 链接静态库
# SDL3 lua
if(WIN32)
    target_link_libraries(sandbox 
        PRIVATE 
            vkRenderer 
            debug "${SDL3_INSTALLED_DIR}/lib/SDL3d.lib"  "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}d.lib"
            optimized "${SDL3_INSTALLED_DIR}/lib/SDL3.lib" "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}.lib"
    )
elseif(UNIX)
    target_link_libraries(sandbox
        PRIVATE
            vkRenderer
            debug "${SDL3_INSTALLED_DIR}/debug/lib/libSDL3.so" "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}d.so"
            optimized "${SDL3_INSTALLED_DIR}/lib/libSDL3.so" "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}.so"
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
# 包含目录
# SDL3 lua sol2(head-only)
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

target_include_directories(
	sandbox
		PRIVATE
			"${LUA_INSTALLED_DIR}/include"
			"${SOL2_INSTALLED_DIR}/include"
)


if(WIN32)
set_target_properties(
    sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
)
else()

endif()