# logger

# 源文件
file(GLOB SOURCES
	"./*.cpp"
	"exception/*.cpp"
)

file(GLOB PUBLIC_HEADERS
	"./*.hpp"
	"exception/*.hpp"
)

#创建静态库
add_library(
	logger
	STATIC ${SOURCES} ${PUBLIC_HEADERS}
)

# 使用utf-8编码编译源文件 仅windows
if(WIN32)
	target_compile_options(logger PRIVATE "/utf-8")
endif()

# 包含目录
target_include_directories(
	logger
		PUBLIC
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/exception"
)