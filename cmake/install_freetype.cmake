# install_freetype.cmake
#
# 用法: cmake -P install_freetype.cmake
#
# 功能:
#   1. 克隆 https://github.com/freetype/freetype.git (master 分支) 到 ./vendor/freetype
#      (仓库已存在时拉取最新 master)
#   2. 用 CMake 分别构建并安装 Debug / Release, 各配置使用独立的安装目录:
#        Debug   -> vendor/freetype/build_debug   安装到 vendor/freetype/dist_debug
#        Release -> vendor/freetype/build_release 安装到 vendor/freetype/dist_release
#
# 可选参数 (通过 -D 传入):
#   FREETYPE_CONFIGS           要构建的配置列表, 默认 "Debug;Release"
#   FREETYPE_INSTALL_DIR_DEBUG   Debug 安装目录, 默认 vendor/freetype/dist_debug
#   FREETYPE_INSTALL_DIR_RELEASE Release 安装目录, 默认 vendor/freetype/dist_release
#   FREETYPE_BUILD_JOBS        并行编译数, 默认不指定 (使用全部核心)
#   FREETYPE_EXTRA_ARGS        追加的 cmake 配置参数
#                              (如 "-DBUILD_SHARED_LIBS=ON" 改为编译动态库)

cmake_minimum_required(VERSION 3.15)

# 脚本位于 ./cmake/ 下, 仓库根目录 (上一级)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(FREETYPE_REPO_URL "https://github.com/freetype/freetype.git")
set(FREETYPE_BRANCH   "master")
set(FREETYPE_SRC_DIR  "${PROJECT_ROOT}/vendor/freetype")

# 各配置独立的安装目录
if(NOT DEFINED FREETYPE_INSTALL_DIR_RELEASE)
    set(FREETYPE_INSTALL_DIR_RELEASE "${FREETYPE_SRC_DIR}/dist_release")
endif()
if(NOT DEFINED FREETYPE_INSTALL_DIR_DEBUG)
    set(FREETYPE_INSTALL_DIR_DEBUG "${FREETYPE_SRC_DIR}/dist_debug")
endif()

if(NOT DEFINED FREETYPE_CONFIGS)
    set(FREETYPE_CONFIGS "Debug;Release")
endif()
if(NOT DEFINED FREETYPE_BUILD_JOBS)
    set(FREETYPE_BUILD_JOBS "")
endif()

# FreeType 配置参数: 默认编译静态库, 关闭 zlib/bzip2/png/brotli/harfbuzz 可选依赖
set(FREETYPE_CMAKE_ARGS
    -DBUILD_SHARED_LIBS=OFF
    -DFT_DISABLE_ZLIB=ON
    -DFT_DISABLE_BZIP2=ON
    -DFT_DISABLE_PNG=ON
    -DFT_DISABLE_BROTLI=ON
    -DFT_DISABLE_HARFBUZZ=ON
)
if(DEFINED FREETYPE_EXTRA_ARGS)
    list(APPEND FREETYPE_CMAKE_ARGS ${FREETYPE_EXTRA_ARGS})
endif()

# ---------- 1. 克隆 / 更新仓库 ----------
if(NOT EXISTS "${FREETYPE_SRC_DIR}/.git")
    message(STATUS "[freetype] 克隆 ${FREETYPE_REPO_URL} (${FREETYPE_BRANCH}) -> ${FREETYPE_SRC_DIR}")
    execute_process(
        COMMAND git clone --branch "${FREETYPE_BRANCH}" --single-branch
                "${FREETYPE_REPO_URL}" "${FREETYPE_SRC_DIR}"
        RESULT_VARIABLE _clone_result
        OUTPUT_VARIABLE _clone_out
        ERROR_VARIABLE  _clone_err
    )
    if(NOT _clone_result EQUAL 0)
        message(FATAL_ERROR "[freetype] git clone 失败:\n${_clone_out}${_clone_err}")
    endif()
else()
    message(STATUS "[freetype] 仓库已存在, 更新到最新 ${FREETYPE_BRANCH}")
    execute_process(
        COMMAND git -C "${FREETYPE_SRC_DIR}" fetch origin "${FREETYPE_BRANCH}"
        RESULT_VARIABLE _fetch_result
        ERROR_VARIABLE  _fetch_err
    )
    if(NOT _fetch_result EQUAL 0)
        message(FATAL_ERROR "[freetype] git fetch 失败:\n${_fetch_err}")
    endif()
    execute_process(
        COMMAND git -C "${FREETYPE_SRC_DIR}" checkout "${FREETYPE_BRANCH}"
        RESULT_VARIABLE _checkout_result
        ERROR_VARIABLE  _checkout_err
    )
    if(NOT _checkout_result EQUAL 0)
        message(FATAL_ERROR "[freetype] git checkout 失败:\n${_checkout_err}")
    endif()
    execute_process(
        COMMAND git -C "${FREETYPE_SRC_DIR}" pull --ff-only
        RESULT_VARIABLE _pull_result
        ERROR_VARIABLE  _pull_err
    )
    if(NOT _pull_result EQUAL 0)
        message(FATAL_ERROR "[freetype] git pull 失败:\n${_pull_err}")
    endif()
endif()

# ---------- 2. 构建并安装各配置 ----------
foreach(_config IN LISTS FREETYPE_CONFIGS)
    string(TOLOWER "${_config}" _config_lc)
    set(_build_dir "${FREETYPE_SRC_DIR}/build_${_config_lc}")

    # 每个配置安装到独立目录
    if(_config STREQUAL "Debug")
        set(_install_dir "${FREETYPE_INSTALL_DIR_DEBUG}")
    else()
        set(_install_dir "${FREETYPE_INSTALL_DIR_RELEASE}")
    endif()

    set(_ft_cmake_args ${FREETYPE_CMAKE_ARGS})
    list(APPEND _ft_cmake_args -DCMAKE_INSTALL_PREFIX=${_install_dir})

    # 配置
    message(STATUS "[freetype] 配置 ${_config} -> ${_build_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${FREETYPE_SRC_DIR}"
            -B "${_build_dir}"
            -DCMAKE_BUILD_TYPE=${_config}
            ${_ft_cmake_args}
        RESULT_VARIABLE _cfg_result
    )
    if(NOT _cfg_result EQUAL 0)
        message(FATAL_ERROR "[freetype] ${_config} 配置失败 (返回码 ${_cfg_result})")
    endif()

    # 编译
    message(STATUS "[freetype] 构建 ${_config}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build "${_build_dir}"
            --parallel ${FREETYPE_BUILD_JOBS}
            --config ${_config}
        RESULT_VARIABLE _build_result
    )
    if(NOT _build_result EQUAL 0)
        message(FATAL_ERROR "[freetype] ${_config} 构建失败 (返回码 ${_build_result})")
    endif()

    # 安装
    message(STATUS "[freetype] 安装 ${_config} -> ${_install_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --install "${_build_dir}"
            --config ${_config}
        RESULT_VARIABLE _install_result
    )
    if(NOT _install_result EQUAL 0)
        message(FATAL_ERROR "[freetype] ${_config} 安装失败 (返回码 ${_install_result})")
    endif()
endforeach()

message(STATUS "[freetype] 完成.")
