# LunarImageIO
# 包裹OpenImageIO

file(GLOB SOURCES
	"./*.cpp"

)

file (GLOB PUBLIC_HEARERS
	"./*.h"
	"./*.hpp"
)


add_library(LunarImageIO STATIC ${SOURCES} ${PUBLIC_HEARERS})

# 链接OpenImageIO
if(${OpenImageIO_FOUND})
	target_link_libraries(LunarImageIO PRIVATE OpenImageIO::OpenImageIO)
else()
	# 使用输入参数
	if(WIN32)
		target_link_libraries(LunarImageIO 
			PRIVATE
				debug "${OIIO_INSTALLED_DIR}/lib/OpenImageIO_d.lib"
				optimized "${OIIO_INSTALLED_DIR}/lib/OpenImageIO.lib"
		)
		target_link_libraries(LunarImageIO 
			PRIVATE 
				debug "${OIIO_INSTALLED_DIR}/lib/OpenImageIO_Util_d.lib"
				optimized "${OIIO_INSTALLED_DIR}/lib/OpenImageIO_Util.lib"
		)
	elseif(UNIX)
		target_link_libraries(LunarImageIO
			PRIVATE 
				"${OIIO_INSTALLED_DIR}/lib/libOpenImageIO.so"
		)
		target_link_libraries(LunarImageIO
			PRIVATE 
				"${OIIO_INSTALLED_DIR}/lib/libOpenImageIO_Util.so"
		)
	else()
		message(FATAL_ERROR "Unsupported platform")
	endif()

	target_include_directories(LunarImageIO 
		PRIVATE 
			"${OIIO_INSTALLED_DIR}/include"
	)

endif()

# 链接logger
target_link_libraries(LunarImageIO PUBLIC logger)

# 使用utf-8编码编译源文件 仅windows
if(WIN32)
	target_compile_options(LunarImageIO PRIVATE "/utf-8")
endif()

# 设置包含目录
target_include_directories(
	LunarImageIO
		PUBLIC
			"${CMAKE_CURRENT_SOURCE_DIR}"
)