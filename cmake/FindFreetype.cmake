# FindFreetype.cmake
#
# 查找 FreeType. 优先使用 cmake/install_freetype.cmake 安装的
# ./vendor/freetype/dist (include/freetype2 + lib + cmake 配置).
#
# 使用方式 (根 CMakeLists.txt):
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#   find_package(Freetype REQUIRED)
#   target_link_libraries(<target> PRIVATE Freetype::Freetype)
#
# 输入变量:
#   FREETYPE_INSTALLED_DIR  安装目录 (含 include/ 与 lib/),
#                           默认 ./vendor/freetype/dist
#   Freetype_ROOT           通用根目录提示 (CMP0074, 由 find_package 传入)
#
# 输出变量:
#   FREETYPE_FOUND
#   FREETYPE_INCLUDE_DIRS
#   FREETYPE_LIBRARY / FREETYPE_LIBRARIES
#   FREETYPE_VERSION_STRING
#   Freetype::Freetype      导入目标

include(FindPackageHandleStandardArgs)

# ---- 搜索提示 ----
if(NOT DEFINED FREETYPE_INSTALLED_DIR)
    set(FREETYPE_INSTALLED_DIR "${CMAKE_CURRENT_LIST_DIR}/../vendor/freetype/dist")
endif()

set(_freetype_hints)
if(EXISTS "${FREETYPE_INSTALLED_DIR}")
    list(APPEND _freetype_hints "${FREETYPE_INSTALLED_DIR}")
endif()

# ---- 头文件 ----
# ft2build.h 与 freetype/config/ftheader.h 通常在 include/freetype2 下
find_path(FREETYPE_INCLUDE_DIR_ft2build
    NAMES ft2build.h
    HINTS ${_freetype_hints}
    PATH_SUFFIXES include include/freetype2)

find_path(FREETYPE_INCLUDE_DIR_freetype
    NAMES freetype/config/ftheader.h
    HINTS ${_freetype_hints}
    PATH_SUFFIXES include include/freetype2)

set(FREETYPE_INCLUDE_DIRS "")
if(FREETYPE_INCLUDE_DIR_ft2build)
    list(APPEND FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_ft2build}")
endif()
if(FREETYPE_INCLUDE_DIR_freetype)
    list(APPEND FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_freetype}")
endif()
list(REMOVE_DUPLICATES FREETYPE_INCLUDE_DIRS)

# ---- 库文件 ----
find_library(FREETYPE_LIBRARY
    NAMES freetype freetyped
    HINTS ${_freetype_hints}
    PATH_SUFFIXES lib lib64)

if(FREETYPE_LIBRARY)
    set(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")
endif()

# ---- 版本号 (从 freetype/freetype.h 提取) ----
if(FREETYPE_INCLUDE_DIR_freetype)
    set(_freetype_header "${FREETYPE_INCLUDE_DIR_freetype}/freetype/freetype.h")
    if(EXISTS "${_freetype_header}")
        foreach(_part MAJOR MINOR PATCH)
            file(STRINGS "${_freetype_header}" _ft_version_line
                 REGEX "^#define[ \t]+FREETYPE_${_part}[ \t]+[0-9]+")
            string(REGEX REPLACE
                   "^#define[ \t]+FREETYPE_${_part}[ \t]+([0-9]+).*" "\\1"
                   FREETYPE_VERSION_${_part} "${_ft_version_line}")
        endforeach()
        if(NOT FREETYPE_VERSION_MAJOR STREQUAL ""
           AND NOT FREETYPE_VERSION_MINOR STREQUAL ""
           AND NOT FREETYPE_VERSION_PATCH STREQUAL "")
            set(FREETYPE_VERSION_STRING
                "${FREETYPE_VERSION_MAJOR}.${FREETYPE_VERSION_MINOR}.${FREETYPE_VERSION_PATCH}")
        endif()
    endif()
endif()

# ---- 结果 ----
find_package_handle_standard_args(Freetype
    REQUIRED_VARS FREETYPE_LIBRARY FREETYPE_INCLUDE_DIR_ft2build FREETYPE_INCLUDE_DIR_freetype
    VERSION_VAR   FREETYPE_VERSION_STRING)

if(Freetype_FOUND AND NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype UNKNOWN IMPORTED)
    set_target_properties(Freetype::Freetype PROPERTIES
        IMPORTED_LOCATION "${FREETYPE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIRS}")
endif()

mark_as_advanced(
    FREETYPE_INCLUDE_DIR_ft2build
    FREETYPE_INCLUDE_DIR_freetype
    FREETYPE_LIBRARY)
