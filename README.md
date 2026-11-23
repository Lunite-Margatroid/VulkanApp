# VulkanApp
Repo to Learn vulkan

# 依赖

## Vulkan

## Slang

## VulkanMemoryAllocator

## OpenImageIO

## SDL3

## glm

## Freeype

## HarfBuzz

## Lua

## sol2





# 构建

设置变量，指定依赖库安装位置。

`XXXX_INSTALLED_DIR` 表示特定库安装的位置

OpenImageIO安装位置`OIIO_INSTALLED_DIR`

glm安装位置 `GLM_INSTALLED_DIR`

SDL安装位置 `SDL3_INSTALLED_DIR`

部分库会使用`find_package`

如果没有找到，使用`XXXX_INSTALLED_DIR`链接。

## Windows

### 安装Freetype

在仓库根目录下运行脚本

```bat
cmake -P cmake/install_freetype.cmake
```

### 安装HarfBuzz

请先按照上文安装Freetype.

然后在仓库根目录下运行脚本

```bat
cmake -P cmake/install_harfbuzz.cmake
```

### 安装OpenImageIO

如果要运行install_oiio.cmake请先按照上文安装Freetype.或者你自己安装OpenImageIO

**手动安装**或者运行脚本：

```bat
cmake -P cmake/install_oiio.cmake
```

如果使用了安装脚本，参数`-D OIIO_INSTALLED_DIR=%CD%/vendor/dist`

### 安装Lua

在仓库根目录下运行脚本

```bat
cmake -P cmake/install_lua.cmake
```

脚本用 `curl` 从 lua.org 官方下载 `lua-5.4.9.tar.gz`（不经过 GitHub），解压到 `vendor/lua/`，分别在 `vendor/lua/build_debug`、`vendor/lua/build_release` 构建，并合并安装到 `vendor/dist/`（Debug 产物带 `d` 后缀，与 Release 共存）。源码已存在且版本一致时跳过下载，版本不一致会自动重新下载。

安装产物：

- `lib/lua54.lib`（Debug 为 `lua54d.lib`，库名版本号从 `lua.h` 自动解析）
- `bin/lua.exe`、`bin/luac.exe`（Debug 为 `luad.exe`、`luacd.exe`）
- `include/lua.h`、`lauxlib.h`、`lualib.h`、`luaconf.h`、`lua.hpp`

网络受限时可通过代理下载（默认使用 curl 的环境变量代理）：

```bat
cmake -P cmake/install_lua.cmake -DLUA_PROXY=http://127.0.0.1:7890
```

其他参数：`LUA_VERSION`（默认 `5.4.9`）、`LUA_INSTALL_DIR`（默认 `vendor/dist`）、`LUA_CONFIGS`（默认 `Debug;Release`）、`LUA_BUILD_JOBS`（并行编译数）。

注意事项：

- 依赖 `curl`（Windows 10 1803+ 自带）。
- Debug/Release 合并安装到 `vendor/dist`（OpenImageIO 的安装脚本也合并到此目录），两组产物通过 `d` 后缀区分，互不冲突。
- Lua 为静态库，Debug 配置使用 `/MDd` 运行时，与项目的 Debug 构建一致。

### 安装sol2

sol2 是 header-only 库，用于后续的脚本/控制台功能（见 `doc/CmdDesign.md`）。

在仓库根目录下运行脚本

```bat
script\install_sol2.bat
```

脚本会从 GitHub 下载 sol2 源码（默认 `v3.5.0`）并解压到 `vendor/sol2/`，同时把公共头文件拷贝到 `vendor/dist/include/sol/`。使用时将 `vendor/dist/include` 加入 include 路径，包含方式为 `#include <sol/sol.hpp>`。

指定版本、覆盖重装：

```bat
script\install_sol2.bat v3.3.0
script\install_sol2.bat v3.5.0 -f
```

网络受限时可通过本地代理下载（也可用环境变量 `SOL2_PROXY` 设置）：

```bat
script\install_sol2.bat -proxy http://127.0.0.1:7890
```

注意事项：

- 依赖 `curl` 和 7-Zip。Windows 10 1803+ 自带 `curl`；7-Zip 需自行安装，脚本会在 PATH 和 `C:\Program Files\7-Zip` 中查找 `7z.exe`。
- `vendor/sol2` 已存在时脚本默认拒绝覆盖，需加 `-f` 参数。
- `vendor/` 已被 `.gitignore` 忽略，下载的源码不会提交到仓库。
- sol2 依赖 Lua，请先按上文安装 Lua（该脚本只下载 sol2）。

### 安装VulkanSDK

使用LunarG提供的安装包安装VulkanSDK.

下载地址：https://vulkan.lunarg.com/sdk/home#windows

其中包含了Vulkan, Slang, VulkanMemoryAllocator, SDL3, glm. 

Slang, VulkanMemoryAllocator, SDL3, glm是可选项，请勾选。

vulkan可以被`find_package`找到。

### 命令

```bat
cmake 	-D OIIO_INSTALLED_DIR=%CD%/vendor/dist ^
		-D VMA_INSTALLED_DIR=%VULKAN_SDK% ^
		-D GLM_INSTALLED_DIR=%VULKAN_SDK% ^
		-D SDL3_INSTALLED_DIR=%VULKAN_SDK% ^
		-D SLANG_INSTALLED_DIR=%VULKAN_SDK% ^
		-D FREETYPE_INSTALLED_DIR=%CD%/vendor/freetype ^
		-D HARFBUZZ_INSTALLED_DIR=%CD%/vendor/harfbuzz ^
		-S ./ -B ./build -G "Visual Studio 17 2022" -A x64 
```



## Linux



