# sandbox

# 源文件
set(SOURCES
    sandbox.cpp
)

# 创建可执行文件
add_executable(sandbox ${SOURCES})

# 链接静态库
target_link_libraries(sandbox PRIVATE vkRenderer "${SDL2_INSTALLED_DIR}/lib/SDL2.lib")

# 设置包含目录
target_include_directories(sandbox 
    PRIVATE 
        "${CMAKE_SOURCE_DIR}/vkRenderer/include"
        "${SDL2_INSTALLED_DIR}/Include"
)