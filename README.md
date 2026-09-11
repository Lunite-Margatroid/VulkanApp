# VulkanApp
Repo to Learn vulkan

# 依赖

## Vulkan



## Slang



## VulkanMemoryAllocator



## OpenImageIO



# SDL3



## glm



# 构建

设置变量，指定依赖库安装位置。

`XXXX_INSTALLED_DIR` 表示特定库安装的位置

OpenImageIO安装位置`OIIO_INSTALLED_DIR`

glm安装位置 `GLM_INSTALLED_DIR`

SDL安装位置 `SDL3_INSTALLED_DIR`

部分库会使用`find_package`

如果没有找到，使用`XXXX_INSTALLED_DIR`链接。

## Windows

### 安装OpenImageIO

**手动安装**或者运行脚本：

```bat
cmake -P cmake/install_oiio.cmake
```

如果使用了安装脚本，参数`-D OIIO_INSTALLED_DIR=%CD%/vendor/dist`

### 安装VulkanSDK

使用LunarG提供的安装包安装VulkanSDK.

下载地址：https://vulkan.lunarg.com/sdk/home#windows

其中包含了Vulkan, Slang, VulkanMemoryAllocator, SDL3, glm

vulkan可以被`find_package`找到。Slang, VulkanMemoryAllocator, SDL3, glm的安装位置都可以写安装地址

### 命令

```bat
cmake 	-D OIIO_INSTALLED_DIR=%CD%/vendor/dist^
		-D VMA_INSTALLED_DIR=%VULKAN_SDK%^
		-D GLM_INSTALLED_DIR=%VULKAN_SDK%^
		-D SDL3_INSTALLED_DIR=%VULKAN_SDK%^
		-D SLANG_INSTALLED_DIR=%VULKAN_SDK%^
		-S ./ -B ./build -G "Visual Studio 17 2022" -A x64 
```



## Linux



