# SlangCompiler

# 源文件
set(SOURCES
	"SlangCompiler.cpp"
)

set(PUBLIC_HEADERS
	"SlangCompiler.hpp"
)

add_library(
	SlangCompiler
	STATIC
	${SOURCES} ${PUBLIC_HEADERS}
)

# 使用utf-8编码编译源文件 仅windows
if(WIN32)
	target_compile_options(SlangCompiler PRIVATE "/utf-8")
endif()

# 链接slang
if(WIN32)
	target_link_libraries(
		SlangCompiler
			PRIVATE
				debug "${SLANG_INSTALLED_DIR}/lib/slangd.lib"
				optimized "${SLANG_INSTALLED_DIR}/lib/slang.lib"
	)
elseif(UNIX)
	target_link_libraries(
		SlangCompiler
			PRIVATE
				"${SLANG_INSTALLED_DIR}/lib/libslang.so"
	)
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

# 链接logger
target_link_libraries(SlangCompiler PUBLIC logger)

# 包含文件
target_include_directories(
	SlangCompiler
		PUBLIC
			"${CMAKE_CURRENT_SOURCE_DIR}"
		PRIVATE
			"${SLANG_INSTALLED_DIR}/include"
)