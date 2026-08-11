# cpydll

# 创建一个专门拷贝 DLL 的目标
add_custom_target(CopyDLLs ALL
    # DEPENDS sandbox
    COMMENT "Copying DLLs"
)

# 拷贝SDL3动态库
if(WIN32)
	if(EXISTS "${SDL3_INSTALLED_DIR}/Bin/SDL3.dll")
		message(STATUS "Copy dll: ${SDL3_INSTALLED_DIR}/Bin/SDL3.dll")

		add_custom_command( TARGET CopyDLLs POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${SDL3_INSTALLED_DIR}/Bin/$<IF:$<CONFIG:Debug>,SDL3d.dll,SDL3.dll>"
			"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL3d.dll,SDL3.dll>"
		COMMENT "Copying SDL3 to output directory" )

	else()
		message(FATAL_ERROR "File does NOT exist: ${SDL3_INSTALLED_DIR}/Bin/SDL3.dll")
	endif()
endif()

# 拷贝slang编译dll
if(WIN32)
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
endif()

# 如果没有找到OpenImageIO，则拷贝OpenImageIO动态库
# 拷贝OpenImageIO动态库
if(WIN32)
	if(NOT ${OpenImageIO_FOUND})

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

	endif()
endif()