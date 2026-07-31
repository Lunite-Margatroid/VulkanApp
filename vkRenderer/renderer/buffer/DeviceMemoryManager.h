#pragma once
#include "Buffer.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"
#include "IndexBuffer.h"
#include "ConstBuffer.h"
#include "DeviceImage.h"
#include "Image2DShaderRes.h"
#include "Image2DDepthBuffer.h"

#define VMA_VULKAN_VERSION 1004000
#include "vk_mem_alloc.h"

namespace LT {
	class DeviceMemoryManager {
	private:
		std::map<BufferID, VmaAllocation> m_mapBufferAllocation;
		std::map<ImageID, VmaAllocation> m_mapImageAllocation;

		VmaAllocator m_vmaDeviceMemAllocator;

		DeviceMemoryManager();
		~DeviceMemoryManager();


		//vk::DeviceMemory AllocateDeviceMemory(vk::MemoryRequirements, vk::MemoryPropertyFlags);

		//void AllocateMemory(Buffer& buffer, vk::MemoryPropertyFlags memoryPorp);

		//void AllocateImageMemory(DeviceImage& image, vk::MemoryPropertyFlags memoryPorp);
	public:
		/// <summary>
		/// 把目标Buffer的Memory映射出来。只有Staging和ConstBuffer有效
		/// </summary>
		/// <param name="buffer"></param>
		/// <returns></returns>
		void* MapMemory(Buffer& buffer);

		/// <summary>
		/// 关闭映射
		/// </summary>
		/// <param name="buffer"></param>
		void UnmapMemory(Buffer& buffer);

	private:
		static DeviceMemoryManager* s_pDeviceMemoryManagerInstance;
	public:
		static bool g_EnableValidationLayer;
		static bool VK_KHR_get_memory_requirements2_enabled;
		static bool VK_KHR_get_physical_device_properties2_enabled;
		static bool VK_KHR_dedicated_allocation_enabled;
		static bool VK_KHR_bind_memory2_enabled;
		static bool VK_EXT_memory_budget_enabled;
		static bool VK_AMD_device_coherent_memory_enabled;
		static bool VK_KHR_buffer_device_address_enabled;
		static bool VK_EXT_memory_priority_enabled;
		static bool VK_EXT_debug_utils_enabled;
		static bool VK_KHR_maintenance5_enabled;
		static bool VK_KHR_external_memory_win32_enabled;
		static bool g_SparseBindingEnabled;

		// 貌似跟VK_KHR_maintenance5_enabled是重复的？
		static bool s_bMaintenance5ExtensionAvailable;
	public:

		// 检查VMA扩展支持
		// 在创建Device之前调用
		// 基本都是从VMA官方的Sample中拷贝的 可能有冗余
		static void CheckVMASupportedExtension(vk::PhysicalDevice physicalDevice);
		static uint32_t GetVkVersion();

		static void Init();
		static void Release();

		static DeviceMemoryManager& GetInstance();

		//static void AllocateMemory(VertexBuffer* vertexBuffer);
		//static void AllocateMemory(StagingBuffer* stagingBuffer);
		//static void AllocateMemory(IndexBuffer* pIndexBuffer);
		//static void AllocateMemory(ConstBuffer* pConstBuffer);
		static void AsignMemory(StagingBuffer* stagingBuffer, size_t nSize, void* pData);
		static void AsignMemory(ConstBuffer* pConstBuffer, size_t nSize, void* pData);
		//static void FreeMemory(Buffer& buffer);


		//static void AllocateMemory(Image2DShaderRes* pImage);
		//static void AllocateMemory(Image2DDepthBuffer* pDepth);

		static void AsignMemory(Image2DShaderRes* pImage, size_t nSize, const void* pData);
		//static void FreeImageMemory(DeviceImage& image);

		static vk::Buffer CreateBuffer(BufferID nBufferID, const vk::BufferCreateInfo& bci,const VmaAllocationCreateInfo& vaci);
		static void ReleaseBuffer(BufferID nBufferID, vk::Buffer vkBuffer);

		static vk::Image CreateImage(ImageID nImageID,const vk::ImageCreateInfo& ici, const VmaAllocationCreateInfo& vaci);
		static void ReleaseImage(ImageID nImageID, vk::Image vkImage);
	};

} // namespace LT