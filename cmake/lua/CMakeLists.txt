# lua 构建工程
#
# 供 cmake/install_lua.cmake 使用 (Lua 官方发行版不包含 CMake 工程).
# 由安装脚本以 -DLUA_SOURCE_DIR=<vendor/lua/src> 配置, 不单独使用.
#
# 产物:
#   静态库 lua<major><minor>.lib, Debug 带 d 后缀 (名字由 lua.h 中的版本号决定)
#   解释器 lua.exe / luad.exe
#   编译器 luac.exe / luacd.exe

cmake_minimum_required(VERSION 3.15)
project(lua C)

if(NOT DEFINED LUA_SOURCE_DIR)
    message(FATAL_ERROR "LUA_SOURCE_DIR 未设置, 请通过 cmake/install_lua.cmake 构建")
endif()

# 从 lua.h 解析版本号, 生成库名 (如 5.5 -> lua55)
file(STRINGS "${LUA_SOURCE_DIR}/lua.h" _lua_major_line REGEX "^#define LUA_VERSION_MAJOR")
file(STRINGS "${LUA_SOURCE_DIR}/lua.h" _lua_minor_line REGEX "^#define LUA_VERSION_MINOR")
string(REGEX MATCH "[0-9]+" _lua_major "${_lua_major_line}")
string(REGEX MATCH "[0-9]+" _lua_minor "${_lua_minor_line}")
set(LUA_LIB_NAME "lua${_lua_major}${_lua_minor}")

# 核心库 = src 下所有 .c, 去掉解释器/编译器的入口文件
file(GLOB LUA_SOURCES "${LUA_SOURCE_DIR}/*.c")
list(REMOVE_ITEM LUA_SOURCES
    "${LUA_SOURCE_DIR}/lua.c"
    "${LUA_SOURCE_DIR}/luac.c"
)

add_library(lua_static STATIC ${LUA_SOURCES})
set_target_properties(lua_static PROPERTIES
    OUTPUT_NAME "${LUA_LIB_NAME}"
    DEBUG_POSTFIX "d"
)
target_include_directories(lua_static PUBLIC "${LUA_SOURCE_DIR}")
if(MSVC)
    target_compile_definitions(lua_static PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
    )
endif()

# 解释器与编译器
add_executable(lua "${LUA_SOURCE_DIR}/lua.c")
target_link_libraries(lua PRIVATE lua_static)

add_executable(luac "${LUA_SOURCE_DIR}/luac.c")
target_link_libraries(luac PRIVATE lua_static)

set_target_properties(lua luac PROPERTIES DEBUG_POSTFIX "d")

include(GNUInstallDirs)

# 安装: bin/lua.exe, luac.exe; lib/lua55.lib; include/公共头文件
install(TARGETS lua_static lua luac
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES
    "${LUA_SOURCE_DIR}/lua.h"
    "${LUA_SOURCE_DIR}/luaconf.h"
    "${LUA_SOURCE_DIR}/lauxlib.h"
    "${LUA_SOURCE_DIR}/lualib.h"
    "${LUA_SOURCE_DIR}/lua.hpp"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
