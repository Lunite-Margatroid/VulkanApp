#define VMA_IMPLEMENTATION

#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DeviceMemoryManager.h"
#include "BufferManager.h"
#include "StagingBuffer.h"

namespace LT {

	DeviceMemoryManager* DeviceMemoryManager::s_pDeviceMemoryManagerInstance = nullptr;

	bool DeviceMemoryManager::g_EnableValidationLayer = true;
	bool DeviceMemoryManager::VK_KHR_get_memory_requirements2_enabled = false;
	bool DeviceMemoryManager::VK_KHR_get_physical_device_properties2_enabled = false;
	bool DeviceMemoryManager::VK_KHR_dedicated_allocation_enabled = false;
	bool DeviceMemoryManager::VK_KHR_bind_memory2_enabled = false;
	bool DeviceMemoryManager::VK_EXT_memory_budget_enabled = false;
	bool DeviceMemoryManager::VK_AMD_device_coherent_memory_enabled = false;
	bool DeviceMemoryManager::VK_KHR_buffer_device_address_enabled = false;
	bool DeviceMemoryManager::VK_EXT_memory_priority_enabled = false;
	bool DeviceMemoryManager::VK_EXT_debug_utils_enabled = false;
	bool DeviceMemoryManager::VK_KHR_maintenance5_enabled = false;
	bool DeviceMemoryManager::VK_KHR_external_memory_win32_enabled = false;
	bool DeviceMemoryManager::g_SparseBindingEnabled = false;

	bool DeviceMemoryManager::s_bMaintenance5ExtensionAvailable = false;


	DeviceMemoryManager& DeviceMemoryManager::GetInstance() {
		RENDERER_ASSERT(s_pDeviceMemoryManagerInstance, "DeviceMemoryManager did not init.");
		return *s_pDeviceMemoryManagerInstance;
	}


	void* DeviceMemoryManager::MapMemory(Buffer& buffer)
	{
		auto iter = m_mapBufferAllocation.find(buffer.GetBufferID());
		if (iter == m_mapBufferAllocation.end())
		{
			LOG_WARNING("%s, Cant find the memory.", __FUNCTION__);
			return nullptr;
		}

		void* pData = nullptr;
		vmaMapMemory(m_vmaDeviceMemAllocator, iter->second, &pData);

		return pData;
	}

	void DeviceMemoryManager::UnmapMemory(Buffer& buffer)
	{
		auto iter = m_mapBufferAllocation.find(buffer.GetBufferID());
		if (iter == m_mapBufferAllocation.end())
		{
			LOG_WARNING("%s, Cant find the memory.", __FUNCTION__);
		}

		vmaUnmapMemory(m_vmaDeviceMemAllocator, iter->second);
	}

	void DeviceMemoryManager::CheckVMASupportedExtension(vk::PhysicalDevice physicalDevice)
	{
		// 检查VMA支持的扩展
		uint32_t nExtensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nExtensionsCount, nullptr);
		std::vector<VkExtensionProperties> vecExtensions(nExtensionsCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nExtensionsCount, vecExtensions.data());



		const uint32_t nVulkanVersion = GetVkVersion();

		for (uint32_t i = 0; i < nExtensionsCount; ++i)
		{
			if (strcmp(vecExtensions[i].extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
			{
				if (nVulkanVersion == VK_API_VERSION_1_0)
				{
					VK_KHR_get_memory_requirements2_enabled = true;
				}
			}
			else if (strcmp(vecExtensions[i].extensionName, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
			{
				if (nVulkanVersion == VK_API_VERSION_1_0)
				{
					VK_KHR_dedicated_allocation_enabled = true;
				}
			}
			else if (strcmp(vecExtensions[i].extensionName, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) == 0)
			{
				if (nVulkanVersion == VK_API_VERSION_1_0)
				{
					VK_KHR_bind_memory2_enabled = true;
				}
			}
			else if (strcmp(vecExtensions[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
				VK_EXT_memory_budget_enabled = true;
			else if (strcmp(vecExtensions[i].extensionName, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) == 0)
				VK_AMD_device_coherent_memory_enabled = true;
			else if (strcmp(vecExtensions[i].extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0)
			{
				if (nVulkanVersion < VK_API_VERSION_1_2)
				{
					VK_KHR_buffer_device_address_enabled = true;
				}
			}
			else if (strcmp(vecExtensions[i].extensionName, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0)
				VK_EXT_memory_priority_enabled = true;
			else if (strcmp(vecExtensions[i].extensionName, VK_KHR_MAINTENANCE_5_EXTENSION_NAME) == 0)
				s_bMaintenance5ExtensionAvailable = true;
			else if (strcmp(vecExtensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME) == 0)
				VK_KHR_external_memory_win32_enabled = VMA_DYNAMIC_VULKAN_FUNCTIONS;
		}


		if (nVulkanVersion >= VK_API_VERSION_1_2)
			VK_KHR_buffer_device_address_enabled = true; // Promoted to core Vulkan 1.2.

		// This sample can use maintenance5 either via core Vulkan 1.4, or via the
		// extension on Vulkan 1.3. It doesn't enable the older dynamic-rendering path.
		s_bMaintenance5ExtensionAvailable =
			nVulkanVersion >= VK_API_VERSION_1_4 ||
			(nVulkanVersion >= VK_API_VERSION_1_3 && s_bMaintenance5ExtensionAvailable);

		// Query for features


#define PnextChainPushFront(mainStruct, newStruct) \
do{\
(newStruct).pNext = (mainStruct).pNext;\
(mainStruct).pNext = &(newStruct);\
} while(false)


//#if VMA_VULKAN_VERSION >= 1001000
//		VkPhysicalDeviceProperties2 physicalDeviceProperties2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
//#endif
//
//#if VMA_VULKAN_VERSION >= 1002000
//		// Vulkan spec says structure VkPhysicalDeviceVulkan11Properties is "Provided by VK_VERSION_1_2" - is this a mistake? Assuming not...
//		VkPhysicalDeviceVulkan11Properties physicalDeviceVulkan11Properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES };
//		VkPhysicalDeviceVulkan12Properties physicalDeviceVulkan12Properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
//		PnextChainPushFront(physicalDeviceProperties2, physicalDeviceVulkan11Properties);
//		PnextChainPushFront(physicalDeviceProperties2, physicalDeviceVulkan12Properties);
//#endif

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

		VkPhysicalDeviceCoherentMemoryFeaturesAMD physicalDeviceCoherentMemoryFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD };
		if (VK_AMD_device_coherent_memory_enabled)
		{
			PnextChainPushFront(physicalDeviceFeatures, physicalDeviceCoherentMemoryFeatures);
		}

		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR physicalDeviceBufferDeviceAddressFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR };
		if (VK_KHR_buffer_device_address_enabled)
		{
			PnextChainPushFront(physicalDeviceFeatures, physicalDeviceBufferDeviceAddressFeatures);
		}

		VkPhysicalDeviceMemoryPriorityFeaturesEXT physicalDeviceMemoryPriorityFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT };
		if (VK_EXT_memory_priority_enabled)
		{
			PnextChainPushFront(physicalDeviceFeatures, physicalDeviceMemoryPriorityFeatures);
		}

		VkPhysicalDeviceMaintenance5FeaturesKHR physicalDeviceMaintenance5Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR };
		if (s_bMaintenance5ExtensionAvailable)
		{
			PnextChainPushFront(physicalDeviceFeatures, physicalDeviceMaintenance5Features);
		}

		vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures);

		g_SparseBindingEnabled = physicalDeviceFeatures.features.sparseBinding != 0;

		// The extension is supported as fake with no real support for this feature? Don't use it.
		if (VK_AMD_device_coherent_memory_enabled && !physicalDeviceCoherentMemoryFeatures.deviceCoherentMemory)
			VK_AMD_device_coherent_memory_enabled = false;
		if (VK_KHR_buffer_device_address_enabled && !physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress)
			VK_KHR_buffer_device_address_enabled = false;
		if (VK_EXT_memory_priority_enabled && !physicalDeviceMemoryPriorityFeatures.memoryPriority)
			VK_EXT_memory_priority_enabled = false;
		VK_KHR_maintenance5_enabled =
			s_bMaintenance5ExtensionAvailable &&
			physicalDeviceMaintenance5Features.maintenance5 != VK_FALSE;

	}

	uint32_t DeviceMemoryManager::GetVkVersion()
	{
#if VMA_VULKAN_VERSION == 1004000
		return VK_API_VERSION_1_4;
#elif VMA_VULKAN_VERSION == 1003000
		return VK_API_VERSION_1_3;
#elif VMA_VULKAN_VERSION == 1002000
		return VK_API_VERSION_1_2;
#elif VMA_VULKAN_VERSION == 1001000
		return VK_API_VERSION_1_1;
#elif VMA_VULKAN_VERSION == 1000000
		return VK_API_VERSION_1_0;
#else
#error Invalid VMA_VULKAN_VERSION.
		return UINT32_MAX;
#endif
	}

	void DeviceMemoryManager::Init() {
		if (!s_pDeviceMemoryManagerInstance)
		{
			s_pDeviceMemoryManagerInstance = new DeviceMemoryManager();
		}
		else
		{
			LOG_WARNING("%s, Repeat Init", __FUNCTION__);
		}
	}

	void DeviceMemoryManager::Release() {
		if (s_pDeviceMemoryManagerInstance) {
			delete s_pDeviceMemoryManagerInstance;
			s_pDeviceMemoryManagerInstance = nullptr;
		}
		else
		{
			LOG_WARNING("%s, it did not init.", __FUNCTION__);
		}
	}

	DeviceMemoryManager::DeviceMemoryManager() {
		VmaAllocatorCreateInfo vaci = {};
		vaci.instance = vkContext::GetVulkanInstance();
		vaci.physicalDevice = vkContext::GetPhysicalDevice();
		vaci.device = vkContext::GetVkDevice();
		vaci.vulkanApiVersion = GetVkVersion();


		if (VK_KHR_dedicated_allocation_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		}
		if (VK_KHR_bind_memory2_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
		}
#if !defined(VMA_MEMORY_BUDGET) || VMA_MEMORY_BUDGET == 1
		if (VK_EXT_memory_budget_enabled && (
			GetVkVersion() >= VK_API_VERSION_1_1 || VK_KHR_get_physical_device_properties2_enabled))
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		}
#endif
		if (VK_AMD_device_coherent_memory_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
		}
		if (VK_KHR_buffer_device_address_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}
#if !defined(VMA_MEMORY_PRIORITY) || VMA_MEMORY_PRIORITY == 1
		if (VK_EXT_memory_priority_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
		}
#endif
		if (VK_KHR_maintenance5_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
		}

		if (VK_KHR_external_memory_win32_enabled)
		{
			vaci.flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
		}

		//if (USE_CUSTOM_CPU_ALLOCATION_CALLBACKS)
		//{
		//	outInfo.pAllocationCallbacks = &g_CpuAllocationCallbacks;
		//}

		vaci.pAllocationCallbacks = nullptr;

		vmaCreateAllocator(&vaci, &m_vmaDeviceMemAllocator);

	}

	DeviceMemoryManager::~DeviceMemoryManager() {
		if (m_mapBufferAllocation.size() > 0 || m_mapImageAllocation.size() > 0) {
			LOG_WARNING("%s, device memory did not free", __FUNCTION__);
		}
		vmaDestroyAllocator(m_vmaDeviceMemAllocator);

	}

	//vk::DeviceMemory DeviceMemoryManager::AllocateDeviceMemory(vk::MemoryRequirements vkMemRequire, vk::MemoryPropertyFlags memoryPorp)
	//{

	//	vk::DeviceMemory memory;
	//	vk::Device& device = vkContext::GetVkDevice();



	//	vk::PhysicalDevice& phyDevice = vkContext::GetPhysicalDevice();
	//	vk::PhysicalDeviceMemoryProperties vkMemProp = phyDevice.getMemoryProperties();

	//	for (uint32_t i = 0; i < vkMemProp.memoryTypeCount; i++) {
	//		if (vkMemRequire.memoryTypeBits & (1 << i)) {
	//			if ((vkMemProp.memoryTypes[i].propertyFlags & memoryPorp) == memoryPorp) {

	//				vk::MemoryAllocateInfo mai;
	//				mai.setAllocationSize(vkMemRequire.size)
	//					.setMemoryTypeIndex(i)
	//					;
	//				memory = device.allocateMemory(mai);

	//				break;
	//			}
	//		}

	//	}
	//	RENDERER_ASSERT(memory, "Memory Allocation Failed.");
	//	return memory;
	//}

	//void DeviceMemoryManager::AllocateMemory(Buffer& buffer, vk::MemoryPropertyFlags memoryPorp)
	//{
	//	BufferID bufferID = buffer.GetBufferID();
	//	vk::Buffer nativeBuffer = buffer.GetNativeBuffer();
	//	vk::Device& device = vkContext::GetVkDevice();

	//	if (m_mapVkMemory.contains(bufferID)) {
	//		LOG_WARNING("%s, the buffer memory has been allocated", __FUNCTION__);
	//		return;
	//	}
	//	// 分配空间
	//	vk::MemoryRequirements vkMemRequire = device.getBufferMemoryRequirements(nativeBuffer);

	//	vk::DeviceMemory memory = AllocateDeviceMemory(vkMemRequire, memoryPorp);
	//	m_mapVkMemory[bufferID] = memory;
	//	device.bindBufferMemory(nativeBuffer, memory, 0);
	//}
	//void DeviceMemoryManager::AllocateImageMemory(DeviceImage& image, vk::MemoryPropertyFlags memoryPorp)
	//{
	//	ImageID imageID = image.GetImageID();
	//	vk::Image nativeImage = image.GetNativeDeviceImage();
	//	vk::Device& device = vkContext::GetVkDevice();

	//	if (m_mapVkImageMemory.contains(imageID)) {
	//		LOG_WARNING("%s, the image memory has been allocated", __FUNCTION__);
	//		return;
	//	}

	//	vk::MemoryRequirements vkMemRequire = device.getImageMemoryRequirements(nativeImage);
	//	vk::DeviceMemory memory = AllocateDeviceMemory(vkMemRequire, memoryPorp);
	//	m_mapVkImageMemory[imageID] = memory;
	//	device.bindImageMemory(nativeImage, memory, 0);

	//}
	//void DeviceMemoryManager::AllocateMemory(VertexBuffer* pVertexBuffer)
	//{
	//	GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pVertexBuffer), vk::MemoryPropertyFlagBits::eDeviceLocal);
	//}
	//void DeviceMemoryManager::AllocateMemory(StagingBuffer* pStagingBuffer)
	//{
	//	GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pStagingBuffer), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	//}
	//void DeviceMemoryManager::AllocateMemory(IndexBuffer* pIndexBuffer)
	//{
	//	GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pIndexBuffer), vk::MemoryPropertyFlagBits::eDeviceLocal);
	//}
	//void DeviceMemoryManager::AllocateMemory(ConstBuffer* pConstBuffer)
	//{
	//	GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pConstBuffer), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	//}
	void DeviceMemoryManager::AsignMemory(StagingBuffer* stagingBuffer, size_t nSize, void* pData)
	{
		auto iter = GetInstance().m_mapBufferAllocation.find(stagingBuffer->GetBufferID());
		if (iter == GetInstance().m_mapBufferAllocation.end())
		{
			LOG_WARNING("%s, the Buffer did not exist", __FUNCTION__);
		}

		RENDERER_ASSERT(stagingBuffer->Size() >= nSize, "out of bounds");

		// 填充
		vk::Device& device = vkContext::GetVkDevice();

		void* pDstData = nullptr;
		vmaMapMemory(GetInstance().m_vmaDeviceMemAllocator, iter->second, &pDstData);

		// void* pDstData = device.mapMemory(iter->second, 0, nSize);
		memcpy(pDstData, pData, nSize);

		vmaUnmapMemory(GetInstance().m_vmaDeviceMemAllocator, iter->second);

		//device.unmapMemory(iter->second);
	}
	void DeviceMemoryManager::AsignMemory(ConstBuffer* pConstBuffer, size_t nSize, void* pData)
	{
		auto iter = GetInstance().m_mapBufferAllocation.find(pConstBuffer->GetBufferID());
		if (iter == GetInstance().m_mapBufferAllocation.end())
		{
			LOG_WARNING("%s, the Buffer did not exist", __FUNCTION__);
		}

		RENDERER_ASSERT(pConstBuffer->Size() >= nSize, "out of bounds");

		// 填充
		vk::Device& device = vkContext::GetVkDevice();
		//void* pDstData = device.mapMemory(iter->second, 0, nSize);

		void* pDstData = nullptr;
		vmaMapMemory(GetInstance().m_vmaDeviceMemAllocator, iter->second, &pDstData);

		memcpy(pDstData, pData, nSize);
		// device.unmapMemory(iter->second);

		vmaUnmapMemory(GetInstance().m_vmaDeviceMemAllocator, iter->second);
	}
	//void DeviceMemoryManager::FreeMemory(Buffer& buffer)
	//{
	//	auto iter = GetInstance().m_mapVkMemory.find(buffer.GetBufferID());
	//	if (iter != GetInstance().m_mapVkMemory.end())
	//	{
	//		vkContext::GetVkDevice().freeMemory(iter->second);
	//		GetInstance().m_mapVkMemory.erase(iter);
	//	}
	//	else
	//	{
	//		LOG_WARNING("%s, the buffer did not allocate", __FUNCTION__);
	//	}
	//}
	//void DeviceMemoryManager::AllocateMemory(Image2DShaderRes* pImage)
	//{
	//	DeviceMemoryManager& instance = GetInstance();
	//	instance.AllocateImageMemory(*reinterpret_cast<DeviceImage*>(pImage), vk::MemoryPropertyFlagBits::eDeviceLocal);
	//}
	//void DeviceMemoryManager::AllocateMemory(Image2DDepthBuffer* pDepth)
	//{
	//	DeviceMemoryManager& instance = GetInstance();
	//	instance.AllocateImageMemory(*reinterpret_cast<DeviceImage*>(pDepth), vk::MemoryPropertyFlagBits::eDeviceLocal);
	//}
	void DeviceMemoryManager::AsignMemory(Image2DShaderRes* pImage, size_t nSize, const void* pData)
	{
		StagingBuffer* pStagingBuffer = BufferManager::CreateStagingBuffer(nSize, pData);

		vk::ImageSubresourceLayers isl;
		isl
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setLayerCount(1)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			;

		std::array<vk::BufferImageCopy, 1> bic;
		bic[0]
			.setBufferOffset(0)
			.setImageOffset(vk::Offset3D(0, 0, 0))
			.setImageExtent(vk::Extent3D(pImage->GetWidth(), pImage->GetHeight(), 1))
			.setImageSubresource(isl)
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			;

		vk::CommandBuffer vkCmdBuffer = vkContext::BeginSingleTimeCmdBuffer();

		vkContext::TransitionImageLayout(
			vkCmdBuffer,
			pImage->GetNativeDeviceImage(),
			vk::PipelineStageFlagBits::eNone,
			vk::PipelineStageFlagBits::eTransfer,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal);

		vkCmdBuffer.copyBufferToImage(
			pStagingBuffer->GetNativeBuffer(),
			pImage->GetNativeDeviceImage(),
			vk::ImageLayout::eTransferDstOptimal,
			bic
		);

		vkContext::TransitionImageLayout(
			vkCmdBuffer,
			pImage->GetNativeDeviceImage(),
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal);

		vkContext::EndSingleTimeCmdBuffer(vkCmdBuffer);


		BufferManager::DeleteBuffer(pStagingBuffer->GetBufferID());
	}
	//void DeviceMemoryManager::FreeImageMemory(DeviceImage& image)
	//{
	//	DeviceMemoryManager& instance = DeviceMemoryManager::GetInstance();
	//	auto iter = instance.m_mapVkImageMemory.find(image.GetImageID());
	//	if (iter == instance.m_mapVkImageMemory.end())
	//	{
	//		LOG_WARNING("%s, the image did not allocate", __FUNCTION__);
	//	}
	//	else
	//	{
	//		vk::Device& device = vkContext::GetVkDevice();
	//		device.freeMemory(iter->second);
	//		instance.m_mapVkImageMemory.erase(iter);
	//	}
	//}
	vk::Buffer DeviceMemoryManager::CreateBuffer(BufferID nBufferID, const vk::BufferCreateInfo& bci, const VmaAllocationCreateInfo& vaci)
	{
		RENDERER_ASSERT(!(GetInstance().m_mapBufferAllocation.contains(nBufferID)), "repeat allocating");

		VkBuffer tBuffer;
		VmaAllocation allocation;

		vmaCreateBuffer(GetInstance().m_vmaDeviceMemAllocator, bci, &vaci, &tBuffer, &allocation, nullptr);

		GetInstance().m_mapBufferAllocation[nBufferID] = allocation;

		return vk::Buffer(tBuffer);
	}
	void DeviceMemoryManager::ReleaseBuffer(BufferID nBufferID, vk::Buffer vkBuffer)
	{
		auto& map = GetInstance().m_mapBufferAllocation;
		auto iter = map.find(nBufferID);
		if (iter != map.end())
		{
			vmaDestroyBuffer(GetInstance().m_vmaDeviceMemAllocator, vkBuffer, iter->second);
			map.erase(iter);
		}
		else
		{
			// WARNING
			LOG_WARNING("Invalid release. The Buffer does not exist.");
		}
	}
	vk::Image DeviceMemoryManager::CreateImage(ImageID nImageID, const vk::ImageCreateInfo& ici, const VmaAllocationCreateInfo& vaci)
	{
		RENDERER_ASSERT(!(GetInstance().m_mapImageAllocation.contains(nImageID)), "repeat allocating");

		VkImage tImage;
		VmaAllocation allocation;
		vmaCreateImage(GetInstance().m_vmaDeviceMemAllocator, ici, &vaci, &tImage, &allocation, nullptr);

		GetInstance().m_mapImageAllocation[nImageID] = allocation;

		return vk::Image(tImage);
	}
	void DeviceMemoryManager::ReleaseImage(ImageID nImageID, vk::Image vkImage)
	{
		auto& map = GetInstance().m_mapImageAllocation;
		auto iter = map.find(nImageID);
		if (iter != map.end())
		{
			vmaDestroyImage(GetInstance().m_vmaDeviceMemAllocator, vkImage, iter->second);
			map.erase(iter);
		}
		else
		{
			LOG_WARNING("Invalid release. The Image does not exist.");
		}
	}
} // namespace LT