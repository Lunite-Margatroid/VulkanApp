# install_oiio
#
# 用法(在项目根目录执行):
#   cmake -P cmake/install_oiio.cmake
#
# 1. 将 OpenImageIO 源码拉取到 vendor/OpenImageIO, 并切换到 dev-3.2 分支
# 2. 分别构建 Debug/Release 到 vendor/OpenImageIO/build_debug、vendor/OpenImageIO/build_release
# 3. 安装到 vendor/dist
#
# 可选参数(通过 -D 传入):
#   OIIO_GENERATOR   生成器, 默认 "Visual Studio 17 2022"
#   OIIO_ARCH        平台, 默认 x64(仅 VS 生成器生效)
#   OIIO_GIT_TAG     分支/标签, 默认 dev-3.2
#   OIIO_CONFIGS     要构建的配置列表, 默认 "Debug;Release"
#   OIIO_BUILD_JOBS  并行编译数, 默认不指定(使用全部核心)
#   OIIO_EXTRA_ARGS  追加的 cmake 配置参数, 如 "-DSTOP_ON_WARNING=0"

cmake_minimum_required(VERSION 3.15)

# ---- 路径(以脚本所在目录为准, 与执行时的 CWD 无关) ----
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(OIIO_SOURCE_DIR  "${PROJECT_ROOT}/vendor/OpenImageIO")
set(OIIO_INSTALL_DIR "${PROJECT_ROOT}/vendor/dist")

# ---- 可覆盖参数 ----
if(NOT DEFINED OIIO_GENERATOR)
    set(OIIO_GENERATOR "Visual Studio 17 2022")
endif()
if(NOT DEFINED OIIO_ARCH)
    set(OIIO_ARCH "x64")
endif()
if(NOT DEFINED OIIO_GIT_URL)
    set(OIIO_GIT_URL "https://github.com/AcademySoftwareFoundation/OpenImageIO.git")
endif()
if(NOT DEFINED OIIO_GIT_TAG)
    set(OIIO_GIT_TAG "dev-3.2")
endif()
if(NOT DEFINED OIIO_CONFIGS)
    set(OIIO_CONFIGS "Debug;Release")
endif()
if(NOT DEFINED OIIO_BUILD_JOBS)
    set(OIIO_BUILD_JOBS "")
endif()

# OIIO 配置参数
set(OIIO_CMAKE_ARGS
    -DUSE_OPENGL=0
    -DUSE_QT=0
    -DUSE_PYTHON=0
    -DOpenImageIO_BUILD_MISSING_DEPS=all
    -DOIIO_BUILD_TESTS=0
    -DCMAKE_INSTALL_PREFIX=${OIIO_INSTALL_DIR}
)
if(DEFINED OIIO_EXTRA_ARGS)
    list(APPEND OIIO_CMAKE_ARGS ${OIIO_EXTRA_ARGS})
endif()

# ---- 多配置生成器(如 VS)用 --config 指定; 单配置生成器(如 Ninja)用 CMAKE_BUILD_TYPE ----
set(_is_multi_config FALSE)
if(OIIO_GENERATOR MATCHES "Visual Studio|Xcode|Ninja Multi-Config")
    set(_is_multi_config TRUE)
endif()

set(_arch_args)
if(OIIO_GENERATOR MATCHES "Visual Studio")
    list(APPEND _arch_args -A "${OIIO_ARCH}")
endif()

# ---- 拉取源码并切换到目标分支 ----
execute_process(COMMAND git --version OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE _git_missing)
if(_git_missing)
    message(FATAL_ERROR "未找到 git, 请先安装并加入 PATH")
endif()

if(EXISTS "${OIIO_SOURCE_DIR}/.git")
    message(STATUS "[OIIO] ${OIIO_SOURCE_DIR} 已存在, 拉取最新并切换到分支 ${OIIO_GIT_TAG}")
    execute_process(
        COMMAND git fetch --all --tags
        WORKING_DIRECTORY "${OIIO_SOURCE_DIR}"
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "git fetch 失败")
    endif()

    # -B 强制将本地分支对齐到远端(有未提交改动时会失败并中止, 不会丢东西)
    execute_process(
        COMMAND git checkout -B "${OIIO_GIT_TAG}" "origin/${OIIO_GIT_TAG}"
        WORKING_DIRECTORY "${OIIO_SOURCE_DIR}"
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "git checkout ${OIIO_GIT_TAG} 失败")
    endif()
else()
    message(STATUS "[OIIO] 克隆 ${OIIO_GIT_URL} (分支 ${OIIO_GIT_TAG}) -> ${OIIO_SOURCE_DIR}")
    execute_process(
        COMMAND git clone --branch "${OIIO_GIT_TAG}" "${OIIO_GIT_URL}" "${OIIO_SOURCE_DIR}"
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "git clone 失败")
    endif()
endif()

# ---- 构建并安装各配置 ----
foreach(_config IN LISTS OIIO_CONFIGS)
    string(TOLOWER "${_config}" _config_lc)
    set(_build_dir "${OIIO_SOURCE_DIR}/build_${_config_lc}")

    set(_build_args)
    set(_cfg_cache_args)
    if(_is_multi_config)
        list(APPEND _build_args --config "${_config}")
    else()
        list(APPEND _cfg_cache_args -DCMAKE_BUILD_TYPE=${_config})
    endif()

    # 配置
    message(STATUS "[OIIO] 配置 ${_config} -> ${_build_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${OIIO_SOURCE_DIR}"
            -B "${_build_dir}"
            -G "${OIIO_GENERATOR}"
            ${_arch_args}
            ${_cfg_cache_args}
            ${OIIO_CMAKE_ARGS}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "OIIO ${_config} 配置失败")
    endif()

    # 编译
    message(STATUS "[OIIO] 构建 ${_config}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build "${_build_dir}"
            --parallel ${OIIO_BUILD_JOBS}
            ${_build_args}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "OIIO ${_config} 构建失败")
    endif()

    # 安装到 vendor/dist
    message(STATUS "[OIIO] 安装 ${_config} -> ${OIIO_INSTALL_DIR}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --install "${_build_dir}"
            ${_build_args}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "OIIO ${_config} 安装失败")
    endif()
endforeach()

message(STATUS "[OIIO] 完成, 安装目录: ${OIIO_INSTALL_DIR}")
