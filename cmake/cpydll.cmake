# cpydll

# 创建一个专门拷贝 DLL 的目标
add_custom_target(CopyDLLs ALL
    # DEPENDS sandbox
    COMMENT "Copying DLLs"
)


if(EXISTS "${SDL2_INSTALLED_DIR}/Bin/SDL2.dll")
    message(STATUS "Copy dll: ${SDL2_INSTALLED_DIR}/Bin/SDL2.dll")

    add_custom_command( TARGET CopyDLLs POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_INSTALLED_DIR}/Bin/SDL2.dll"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SDL2.dll"
    COMMENT "Copying SDL2 to output directory" )
else()
    message(FATAL_ERROR "File does NOT exist: ${SDL2_INSTALLED_DIR}/Bin/SDL2.dll")
endif()