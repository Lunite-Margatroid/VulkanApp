#cpyslang

add_custom_target(CopySlang ALL
	COMMENT "Copy Slang file"
)

if(EXISTS "${PROJECT_SOURCE_DIR}/vkRenderer/slang")
	message(STATUS "Copy slang file")
	add_custom_command(
		TARGET CopySlang
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/vkRenderer/slang"
		"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/slang"
		COMMENT "Copying slang files to output dir"
	)
else()
	message(FATAL_ERROR "slang dir does not exists.")
endif()