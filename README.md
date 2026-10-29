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



