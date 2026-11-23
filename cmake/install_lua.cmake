# install_lua.cmake
#
# 用法(在项目根目录执行):
#   cmake -P cmake/install_lua.cmake
#
# 功能:
#   1. 从 lua.org 官方下载 lua-<version>.tar.gz 并解压到 vendor/lua
#      (源码已存在时跳过下载)
#   2. 用 cmake/lua/CMakeLists.txt 分别构建 Debug / Release:
#        Debug   -> vendor/lua/build_debug   安装到 vendor/dist
#        Release -> vendor/lua/build_release 安装到 vendor/dist
#      两个配置合并安装到同一目录, 通过 d 后缀区分 Debug 产物
#
# 产物 (vendor/dist):
#   include/lua.h 等公共头文件
#   lib/lua<major><minor>.lib  (Debug 带 d 后缀, 如 lua54.lib / lua54d.lib)
#   bin/lua.exe      (Debug 为 luad.exe)
#   bin/luac.exe     (Debug 为 luacd.exe)
#
# 可选参数(通过 -D 传入):
#   LUA_VERSION     版本号, 默认 5.4.9
#   LUA_URL         下载地址, 默认 https://www.lua.org/ftp/lua-<version>.tar.gz
#   LUA_PROXY       curl 代理地址, 如 http://127.0.0.1:7890 (默认读取环境变量)
#   LUA_INSTALL_DIR 合并安装目录, 默认 vendor/dist
#   LUA_CONFIGS     要构建的配置列表, 默认 "Debug;Release"
#   LUA_BUILD_JOBS  并行编译数, 默认不指定 (使用全部核心)
#   LUA_EXTRA_ARGS  追加的 cmake 配置参数

cmake_minimum_required(VERSION 3.15)

# 脚本位于 ./cmake/ 下, 仓库根目录 (上一级)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# ---- 可覆盖参数 ----
if(NOT DEFINED LUA_VERSION)
    set(LUA_VERSION "5.4.9")
endif()
if(NOT DEFINED LUA_URL)
    set(LUA_URL "https://www.lua.org/ftp/lua-${LUA_VERSION}.tar.gz")
endif()
if(NOT DEFINED LUA_INSTALL_DIR)
    set(LUA_INSTALL_DIR "${PROJECT_ROOT}/vendor/dist")
endif()
if(NOT DEFINED LUA_CONFIGS)
    set(LUA_CONFIGS "Debug;Release")
endif()
if(NOT DEFINED LUA_BUILD_JOBS)
    set(LUA_BUILD_JOBS "")
endif()

set(LUA_SOURCE_DIR "${PROJECT_ROOT}/vendor/lua")
set(LUA_SRC_DIR    "${LUA_SOURCE_DIR}/src")
set(LUA_CMAKE_DIR  "${PROJECT_ROOT}/cmake/lua")

# ---------- 1. 下载并解压 ----------
# 检查现有源码的版本; 与请求版本不一致时重新下载
set(_have_source FALSE)
if(EXISTS "${LUA_SRC_DIR}/lua.h")
    file(STRINGS "${LUA_SRC_DIR}/lua.h" _major_line REGEX "^#define LUA_VERSION_MAJOR")
    file(STRINGS "${LUA_SRC_DIR}/lua.h" _minor_line REGEX "^#define LUA_VERSION_MINOR")
    file(STRINGS "${LUA_SRC_DIR}/lua.h" _release_line REGEX "^#define LUA_VERSION_RELEASE")
    string(REGEX MATCH "[0-9]+" _major "${_major_line}")
    string(REGEX MATCH "[0-9]+" _minor "${_minor_line}")
    string(REGEX MATCH "[0-9]+" _release "${_release_line}")
    set(_src_version "${_major}.${_minor}.${_release}")
    if(_src_version STREQUAL "${LUA_VERSION}")
        set(_have_source TRUE)
        message(STATUS "[lua] 源码已存在 (${_src_version}), 跳过下载: ${LUA_SOURCE_DIR}")
    else()
        message(STATUS "[lua] 现有源码版本 ${_src_version} 与请求的 ${LUA_VERSION} 不一致, 重新下载")
        # 主次版本号变化时库文件名不同, 记录旧名字用于清理
        set(_old_lib_name "lua${_major}${_minor}")
    endif()
endif()

if(NOT _have_source)
    execute_process(COMMAND curl --version OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE _curl_missing)
    if(_curl_missing)
        message(FATAL_ERROR "[lua] 未找到 curl, 请先安装并加入 PATH")
    endif()

    set(_archive "${PROJECT_ROOT}/vendor/lua-${LUA_VERSION}.tar.gz")
    set(_curl_args -L --fail --retry 3 --retry-all-errors --connect-timeout 15 -sS -o "${_archive}")
    if(DEFINED LUA_PROXY AND NOT LUA_PROXY STREQUAL "")
        list(APPEND _curl_args -x "${LUA_PROXY}")
    endif()

    message(STATUS "[lua] 下载 ${LUA_URL}")
    execute_process(
        COMMAND curl ${_curl_args} "${LUA_URL}"
        RESULT_VARIABLE _res
    )
    if(_res)
        file(REMOVE "${_archive}")
        message(FATAL_ERROR "[lua] 下载失败 (curl 返回码 ${_res})")
    endif()

    # 解压到 vendor/, 得到 vendor/lua-<version>/, 再重命名为 vendor/lua
    message(STATUS "[lua] 解压 ${_archive}")
    set(_extracted "${PROJECT_ROOT}/vendor/lua-${LUA_VERSION}")
    file(REMOVE_RECURSE "${_extracted}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${_archive}"
        WORKING_DIRECTORY "${PROJECT_ROOT}/vendor"
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "[lua] 解压失败")
    endif()
    if(NOT EXISTS "${_extracted}/src/lua.h")
        message(FATAL_ERROR "[lua] 解压后未找到 ${_extracted}/src/lua.h")
    endif()

    if(EXISTS "${LUA_SOURCE_DIR}")
        message(STATUS "[lua] 删除旧源码目录 ${LUA_SOURCE_DIR}")
        file(REMOVE_RECURSE "${LUA_SOURCE_DIR}")
    endif()
    file(RENAME "${_extracted}" "${LUA_SOURCE_DIR}")
    file(REMOVE "${_archive}")
    message(STATUS "[lua] 源码就绪: ${LUA_SOURCE_DIR}")

    # 清理旧主次版本的库文件 (如 lua55.lib / lua55d.lib)
    if(DEFINED _old_lib_name)
        file(GLOB _stale_libs "${LUA_INSTALL_DIR}/lib/${_old_lib_name}*.lib")
        foreach(_lib IN LISTS _stale_libs)
            message(STATUS "[lua] 删除旧版本库 ${_lib}")
            file(REMOVE "${_lib}")
        endforeach()
    endif()
endif()

# ---------- 2. 构建并安装各配置 ----------
foreach(_config IN LISTS LUA_CONFIGS)
    string(TOLOWER "${_config}" _config_lc)
    set(_build_dir "${LUA_SOURCE_DIR}/build_${_config_lc}")

    set(_lua_cmake_args
        -DLUA_SOURCE_DIR=${LUA_SRC_DIR}
        -DCMAKE_INSTALL_PREFIX=${LUA_INSTALL_DIR}
    )
    if(DEFINED LUA_EXTRA_ARGS)
        list(APPEND _lua_cmake_args ${LUA_EXTRA_ARGS})
    endif()

    # 配置
    message(STATUS "[lua] 配置 ${_config} -> ${_build_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${LUA_CMAKE_DIR}"
            -B "${_build_dir}"
            -DCMAKE_BUILD_TYPE=${_config}
            ${_lua_cmake_args}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "[lua] ${_config} 配置失败 (返回码 ${_res})")
    endif()

    # 编译
    message(STATUS "[lua] 构建 ${_config}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build "${_build_dir}"
            --parallel ${LUA_BUILD_JOBS}
            --config ${_config}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "[lua] ${_config} 构建失败 (返回码 ${_res})")
    endif()

    # 安装
    message(STATUS "[lua] 安装 ${_config} -> ${LUA_INSTALL_DIR}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --install "${_build_dir}"
            --config ${_config}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "[lua] ${_config} 安装失败 (返回码 ${_res})")
    endif()
endforeach()

message(STATUS "[lua] 完成.")
