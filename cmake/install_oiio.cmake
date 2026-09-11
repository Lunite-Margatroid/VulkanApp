# install_oiio
#
# 用法(在项目根目录执行):
#   cmake -P cmake/install_oiio.cmake
#
# 1. 将 OpenImageIO 源码拉取到 vendor/OpenImageIO, 并切换到 dev-3.2 分支
# 2. 分别构建 Debug/Release 到 vendor/OpenImageIO/build_debug、vendor/OpenImageIO/build_release
# 3. 分别安装到 vendor/dist_debug、vendor/dist_release, 再合并拷贝到 vendor/dist
#
# 可选参数(通过 -D 传入):
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
)
if(DEFINED OIIO_EXTRA_ARGS)
    list(APPEND OIIO_CMAKE_ARGS ${OIIO_EXTRA_ARGS})
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
    list(APPEND _build_args --config "${_config}")
    list(APPEND _cfg_cache_args -DCMAKE_BUILD_TYPE=${_config})

	# 安装目录
	# release安装到dist_release
	# debug安装到dist_debug
	set(OIIO_INSTALL_DIR_CONFIG "${OIIO_INSTALL_DIR}_${_config_lc}")
	set(_oiio_cmake_args ${OIIO_CMAKE_ARGS})
	list(APPEND _oiio_cmake_args -DCMAKE_INSTALL_PREFIX=${OIIO_INSTALL_DIR_CONFIG})

    # 配置
    message(STATUS "[OIIO] 配置 ${_config} -> ${_build_dir}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${OIIO_SOURCE_DIR}"
            -B "${_build_dir}"
            ${_cfg_cache_args}
            ${_oiio_cmake_args}
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
    message(STATUS "[OIIO] 安装 ${_config} -> ${_oiio_cmake_args}")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --install "${_build_dir}"
            ${_build_args}
        RESULT_VARIABLE _res
    )
    if(_res)
        message(FATAL_ERROR "OIIO ${_config} 安装失败")
    endif()

    # 补拷依赖暂存目录(deps/dist): OIIO 的 install 规则只在"构建该依赖的当次配置"里
    # 注册(dependency_utils.cmake 的 checked_find_package 后续会 find 到已装好的
    # deps/dist 并跳过本地构建分支), 中途中断后重跑会漏装已构建依赖的 dll/lib。
    # 这里直接把 deps/dist 的产物补进安装目录, 代价小且幂等
    set(_deps_dist "${_build_dir}/deps/dist")
    if(EXISTS "${_deps_dist}")
        if(EXISTS "${_deps_dist}/bin")
            message(STATUS "[OIIO] 补拷依赖 dll: ${_deps_dist}/bin -> ${OIIO_INSTALL_DIR_CONFIG}/bin")
            file(COPY "${_deps_dist}/bin/" DESTINATION "${OIIO_INSTALL_DIR_CONFIG}/bin"
                 FILES_MATCHING PATTERN "*.dll")
        endif()
        if(EXISTS "${_deps_dist}/lib")
            message(STATUS "[OIIO] 补拷依赖 lib: ${_deps_dist}/lib -> ${OIIO_INSTALL_DIR_CONFIG}/lib")
            file(COPY "${_deps_dist}/lib/" DESTINATION "${OIIO_INSTALL_DIR_CONFIG}/lib"
                 FILES_MATCHING PATTERN "*.lib")
        endif()
    endif()
endforeach()

# ---- 合并 dist_debug 与 dist_release 到 dist ----
# 先拷 debug 再拷 release: 同名文件(exe、第三方 dll/lib)以 release 为准,
# debug 独有的 *_d.dll / *_d.lib 不被覆盖, 两个配置的产物共存于同一目录树
file(REMOVE_RECURSE "${OIIO_INSTALL_DIR}")
file(MAKE_DIRECTORY "${OIIO_INSTALL_DIR}")
foreach(_src IN ITEMS dist_debug dist_release)
    set(_src_dir "${PROJECT_ROOT}/vendor/${_src}")
    if(EXISTS "${_src_dir}")
        message(STATUS "[OIIO] 合并拷贝 ${_src} -> ${OIIO_INSTALL_DIR}")
        file(COPY "${_src_dir}/" DESTINATION "${OIIO_INSTALL_DIR}")
    endif()
endforeach()

message(STATUS "[OIIO] 完成, 安装目录: ${OIIO_INSTALL_DIR}")
