# install_harfbuzz.cmake
#
# 用法: cmake -P install_harfbuzz.cmake
#
# 功能:
#   1. 克隆 https://github.com/harfbuzz/harfbuzz.git (main 分支) 到 ./vendor/harfbuzz
#      (仓库已存在时拉取最新 main)
#   2. 用 CMake 分别构建并安装 Debug / Release, 各配置使用独立的安装目录:
#        Debug   -> vendor/harfbuzz/build_debug   安装到 vendor/harfbuzz/dist_debug
#        Release -> vendor/harfbuzz/build_release 安装到 vendor/harfbuzz/dist_release
#
# 注意: 启用了 HB_HAVE_FREETYPE, 依赖对应配置的 freetype 安装目录
#       (vendor/freetype/dist_debug|dist_release), 需先运行 install_freetype.cmake,
#       由 cmake/FindFreetype.cmake 查找.
#
# 可选参数 (通过 -D 传入):
#   HARFBUZZ_CONFIGS     要构建的配置列表, 默认 "Debug;Release"
#   HARFBUZZ_BUILD_JOBS  并行编译数, 默认不指定 (使用全部核心)
#   HARFBUZZ_EXTRA_ARGS  追加的 cmake 配置参数

cmake_minimum_required(VERSION 3.15)

# 脚本位于 ./cmake/ 下, 仓库根目录 (上一级)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(HARFBUZZ_REPO_URL "https://github.com/harfbuzz/harfbuzz.git")
set(HARFBUZZ_BRANCH   "main")
set(HARFBUZZ_SRC_DIR  "${PROJECT_ROOT}/vendor/harfbuzz")

if(NOT DEFINED HARFBUZZ_CONFIGS)
    set(HARFBUZZ_CONFIGS "Debug;Release")
endif()
if(NOT DEFINED HARFBUZZ_BUILD_JOBS)
    set(HARFBUZZ_BUILD_JOBS "")
endif()

# HarfBuzz 配置参数: 只编译核心静态库,
# 关闭工具/子库 (subset/raster/vector/gpu), 仅启用 freetype 互操作 (hb-ft)
set(HARFBUZZ_CMAKE_ARGS
    -DBUILD_SHARED_LIBS=OFF
    -DHB_BUILD_UTILS=OFF
    -DHB_BUILD_SUBSET=OFF
    -DHB_BUILD_RASTER=OFF
    -DHB_BUILD_VECTOR=OFF
    -DHB_BUILD_GPU=OFF
    -DHB_HAVE_FREETYPE=ON
)
if(DEFINED HARFBUZZ_EXTRA_ARGS)
    list(APPEND HARFBUZZ_CMAKE_ARGS ${HARFBUZZ_EXTRA_ARGS})
endif()

# ---------- 1. 克隆 / 更新仓库 ----------
if(NOT EXISTS "${HARFBUZZ_SRC_DIR}/.git")
    message(STATUS "[harfbuzz] 克隆 ${HARFBUZZ_REPO_URL} (${HARFBUZZ_BRANCH}) -> ${HARFBUZZ_SRC_DIR}")
    execute_process(
        COMMAND git clone --branch "${HARFBUZZ_BRANCH}" --single-branch
                "${HARFBUZZ_REPO_URL}" "${HARFBUZZ_SRC_DIR}"
        RESULT_VARIABLE _clone_result
        OUTPUT_VARIABLE _clone_out
        ERROR_VARIABLE  _clone_err
    )
    if(NOT _clone_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] git clone 失败:\n${_clone_out}${_clone_err}")
    endif()
else()
    message(STATUS "[harfbuzz] 仓库已存在, 更新到最新 ${HARFBUZZ_BRANCH}")
    execute_process(
        COMMAND git -C "${HARFBUZZ_SRC_DIR}" fetch origin "${HARFBUZZ_BRANCH}"
        RESULT_VARIABLE _fetch_result
        ERROR_VARIABLE  _fetch_err
    )
    if(NOT _fetch_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] git fetch 失败:\n${_fetch_err}")
    endif()
    execute_process(
        COMMAND git -C "${HARFBUZZ_SRC_DIR}" checkout "${HARFBUZZ_BRANCH}"
        RESULT_VARIABLE _checkout_result
        ERROR_VARIABLE  _checkout_err
    )
    if(NOT _checkout_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] git checkout 失败:\n${_checkout_err}")
    endif()
    execute_process(
        COMMAND git -C "${HARFBUZZ_SRC_DIR}" pull --ff-only
        RESULT_VARIABLE _pull_result
        ERROR_VARIABLE  _pull_err
    )
    if(NOT _pull_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] git pull 失败:\n${_pull_err}")
    endif()
endif()

# ---------- 2. 构建并安装各配置 ----------
foreach(_config IN LISTS HARFBUZZ_CONFIGS)
    string(TOLOWER "${_config}" _config_lc)
    set(_build_dir   "${HARFBUZZ_SRC_DIR}/build_${_config_lc}")
    set(_install_dir "${HARFBUZZ_SRC_DIR}/dist_${_config_lc}")

    set(_hb_cmake_args ${HARFBUZZ_CMAKE_ARGS})
    list(APPEND _hb_cmake_args -DCMAKE_INSTALL_PREFIX=${_install_dir})
    # 使用项目自带的查找模块 (FindFreetype.cmake)
    list(APPEND _hb_cmake_args -DCMAKE_MODULE_PATH=${PROJECT_ROOT}/cmake)
    # 清除上次配置缓存的 freetype 查找结果, 避免安装目录变化后仍指向旧路径
    list(APPEND _hb_cmake_args
        -UFREETYPE_LIBRARY
        -UFREETYPE_INCLUDE_DIR_ft2build
        -UFREETYPE_INCLUDE_DIR_freetype)
    # freetype: 对应配置的独立安装目录
    list(APPEND _hb_cmake_args -DFREETYPE_INSTALLED_DIR=${PROJECT_ROOT}/vendor/freetype/dist_${_config_lc})

    # 配置
    message(STATUS "[harfbuzz] 配置 ${_config} -> ${_build_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${HARFBUZZ_SRC_DIR}"
            -B "${_build_dir}"
            -DCMAKE_BUILD_TYPE=${_config}
            ${_hb_cmake_args}
        RESULT_VARIABLE _cfg_result
    )
    if(NOT _cfg_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] ${_config} 配置失败 (返回码 ${_cfg_result})")
    endif()

    # 编译
    message(STATUS "[harfbuzz] 构建 ${_config}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build "${_build_dir}"
            --parallel ${HARFBUZZ_BUILD_JOBS}
            --config ${_config}
        RESULT_VARIABLE _build_result
    )
    if(NOT _build_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] ${_config} 构建失败 (返回码 ${_build_result})")
    endif()

    # 安装
    message(STATUS "[harfbuzz] 安装 ${_config} -> ${_install_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --install "${_build_dir}"
            --config ${_config}
        RESULT_VARIABLE _install_result
    )
    if(NOT _install_result EQUAL 0)
        message(FATAL_ERROR "[harfbuzz] ${_config} 安装失败 (返回码 ${_install_result})")
    endif()
endforeach()

message(STATUS "[harfbuzz] 完成.")
