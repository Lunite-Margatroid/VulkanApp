# LunarTextIO

# 源文件
file(GLOB SOURCES
	"./*.cpp"
)

# 头文件

file(GLOB PUBLIC_HEADERS
	"./*.hpp"
)


# 添加筛选器
# 根据目录自动分组
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
             FILES ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})
			 
# 创建静态库
add_library(
	LunarTextIO STATIC
	${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)

# 链接logger
target_link_libraries(LunarTextIO PUBLIC logger)

# 链接FreeType和harfbuzz
if(WIN32)
	target_link_libraries(
		LunarTextIO
			PRIVATE
				debug 
					"${FREETYPE_INSTALLED_DIR}/dist_debug/lib/freetyped.lib"
					"${HARFBUZZ_INSTALLED_DIR}/dist_debug/lib/harfbuzz.lib"
						
				optimized 
					"${FREETYPE_INSTALLED_DIR}/dist_release/lib/freetype.lib"
					"${HARFBUZZ_INSTALLED_DIR}/dist_release/lib/harfbuzz.lib"
	)
	
	target_include_directories(
	LunarTextIO
		PRIVATE
			debug
				"${FREETYPE_INSTALLED_DIR}/dist_debug/include"
				"${FREETYPE_INSTALLED_DIR}/dist_debug/include/freetype2"
				"${HARFBUZZ_INSTALLED_DIR}/dist_debug/include"
			optimized
				"${FREETYPE_INSTALLED_DIR}/dist_release/include"
				"${FREETYPE_INSTALLED_DIR}/dist_release/include/freetype2"
				"${HARFBUZZ_INSTALLED_DIR}/dist_release/include"
	)
	
elseif(UNIX)
	target_link_libraries(
		LunarTextIO
			PRIVATE
				"${FREETYPE_INSTALLED_DIR}/lib/freetype.so"
				"${HARFBUZZ_INSTALLED_DIR}/lib/harfbuzz.so"
	)
	
	target_include_directories(
	LunarTextIO
		PRIVATE
			"${FREETYPE_INSTALLED_DIR}/include"
			"${FREETYPE_INSTALLED_DIR}/include/freetype2"
			"${HARFBUZZ_INSTALLED_DIR}/include"
)
	
else()
	message(FATAL_ERROR "Unsupported platform")
endif()




# 使用utf-8编码编译源文件 仅windows
if(WIN32)
	target_compile_options(LunarTextIO PRIVATE "/utf-8")
endif()

# 设置包含目录
target_include_directories(
	LunarTextIO
		PUBLIC
			"${CMAKE_CURRENT_SOURCE_DIR}"
)