# cpyasset.cmake

add_custom_target(CopyAsset ALL
	COMMENT "Copy Asset File"
)

if(EXISTS "${PROJECT_SOURCE_DIR}/TestAsset")
	message(STATUS "Copy Asset File")
	add_custom_command(
		TARGET CopyAsset
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/TestAsset"
		"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/TestAsset"
		COMMENT "Coping Asset File to output Dir"
	)
else()
	message(WARNING "Cant find TestAsset Dir")
endif()
