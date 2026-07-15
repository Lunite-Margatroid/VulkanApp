# cpydll

# 创建一个专门拷贝 DLL 的目标
add_custom_target(CopyDLLs ALL
    # DEPENDS sandbox
    COMMENT "Copying DLLs"
)

# 拷贝SDL2动态库
if(EXISTS "${SDL2_INSTALLED_DIR}/Bin/SDL2.dll")
    message(STATUS "Copy dll: ${SDL2_INSTALLED_DIR}/Bin/SDL2.dll")

	add_custom_command( TARGET CopyDLLs POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	    "${SDL2_INSTALLED_DIR}/Bin/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
	    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
	COMMENT "Copying SDL2 to output directory" )

else()
    message(FATAL_ERROR "File does NOT exist: ${SDL2_INSTALLED_DIR}/Bin/SDL2.dll")
endif()

# 拷贝slang编译dll
if(EXISTS "${ENV_VULKAN_PATH}/Bin/slang.dll")
	message(STATUS "Copy dll: ${ENV_VULKAN_PATH}/Bin/slang.dll  glslang.dll")

		add_custom_command(
			TARGET CopyDLLs
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${ENV_VULKAN_PATH}/Bin/$<IF:$<CONFIG:Debug>,slangd.dll,slang.dll>"
				"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/$<IF:$<CONFIG:Debug>,slangd.dll,slang.dll>"
			COMMENT "Coping slang.dll to output directory"
		)

		add_custom_command(
			TARGET CopyDLLs
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${ENV_VULKAN_PATH}/Bin/$<IF:$<CONFIG:Debug>,glslangd.dll,glslang.dll>"
				"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/$<IF:$<CONFIG:Debug>,glslangd.dll,glslang.dll>"
			COMMENT "Coping glslang.dll to output directory"
		)

else()
	message(FATAL_ERROR, "File does NOT exist: ${ENV_VULKAN_PATH}/Bin/slang.dll  glslang.dll")
endif()


# 拷贝OpenImageIO动态库
file(GLOB OIIO_DLLS "${OIIO_INSTALLED_DIR}/bin/*.dll")
if(NOT EXISTS "${OIIO_INSTALLED_DIR}/bin/OpenImageIO.dll")
	message(FATAL_ERROR, "OpenImageIO Did not exist")
endif()

foreach(DLL ${OIIO_DLLS})
	message(STATUS "Found OpenImageIO DLL: ${dll}")
	# 构建目标文件路径
	get_filename_component(DLL_NAME ${DLL} NAME)
	set(DEST_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${DLL_NAME}")


	message(STATUS "Copy dll: ${DLL}")
	
	add_custom_command(
		TARGET CopyDLLs
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${DLL}"
			"${DEST_FILE}"
		COMMENT "Copying ${DLL} to output directory"
	)

endforeach()
