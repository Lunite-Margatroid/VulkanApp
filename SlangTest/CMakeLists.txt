# SlangTest

# 源文件
set(
	SOURCES
	SlangTest.cpp
)

# 创建可执行文件
add_executable(SlangTest ${SOURCES})

if(WIN32)
	target_compile_options(SlangTest PRIVATE "/utf-8")
endif()

# 链接 vkRenderer 和 SlangCompiler
target_link_libraries(
	SlangTest 
		PRIVATE
			vkRenderer
			SlangCompiler
			)
			
# 设置包含目录
target_include_directories(
	SlangTest
		PRIVATE
			"${CMAKE_SOURCE_DIR}"
)

if(WIN32)
	set_target_properties(
		SlangTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
	)
else()

endif()