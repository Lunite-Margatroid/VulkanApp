#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DeviceMemoryManager.h"
#include "BufferManager.h"
#include "StagingBuffer.h"

namespace LT {

	DeviceMemoryManager* DeviceMemoryManager::s_pDeviceMemoryManagerInstance = nullptr;

	DeviceMemoryManager& DeviceMemoryManager::GetInstance() {
		RENDERER_ASSERT(s_pDeviceMemoryManagerInstance, "DeviceMemoryManager did not init.");
		return *s_pDeviceMemoryManagerInstance;
	}


	void* DeviceMemoryManager::MapMemory(Buffer& buffer)
	{
		auto iter = m_mapVkMemory.find(buffer.GetBufferID());
		if (iter == m_mapVkMemory.end())
		{
			LOG_WARNING("%s, Cant find the memory.", __FUNCTION__);
			return nullptr;
		}
		return vkContext::GetVkDevice().mapMemory(iter->second, 0, buffer.Size());
	}

	void DeviceMemoryManager::UnmapMemory(Buffer& buffer)
	{
		auto iter = m_mapVkMemory.find(buffer.GetBufferID());
		if (iter == m_mapVkMemory.end())
		{
			LOG_WARNING("%s, Cant find the memory.", __FUNCTION__);
		}
		vkContext::GetVkDevice().unmapMemory(iter->second);
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

	}

	DeviceMemoryManager::~DeviceMemoryManager() {
		if (m_mapVkMemory.size() > 0) {
			LOG_WARNING("%s, device memory did not free", __FUNCTION__);
		}
	}

	vk::DeviceMemory DeviceMemoryManager::AllocateDeviceMemory(vk::MemoryRequirements vkMemRequire, vk::MemoryPropertyFlags memoryPorp)
	{

		vk::DeviceMemory memory;
		vk::Device& device = vkContext::GetVkDevice();



		vk::PhysicalDevice& phyDevice = vkContext::GetPhysicalDevice();
		vk::PhysicalDeviceMemoryProperties vkMemProp = phyDevice.getMemoryProperties();

		for (uint32_t i = 0; i < vkMemProp.memoryTypeCount; i++) {
			if (vkMemRequire.memoryTypeBits & (1 << i)) {
				if ((vkMemProp.memoryTypes[i].propertyFlags & memoryPorp) == memoryPorp) {

					vk::MemoryAllocateInfo mai;
					mai.setAllocationSize(vkMemRequire.size)
						.setMemoryTypeIndex(i)
						;
					memory = device.allocateMemory(mai);

					break;
				}
			}

		}
		RENDERER_ASSERT(memory, "Memory Allocation Failed.");
		return memory;
	}

	void DeviceMemoryManager::AllocateMemory(Buffer& buffer, vk::MemoryPropertyFlags memoryPorp)
	{
		BufferID bufferID = buffer.GetBufferID();
		vk::Buffer nativeBuffer = buffer.GetNativeBuffer();
		vk::Device& device = vkContext::GetVkDevice();

		if (m_mapVkMemory.contains(bufferID)) {
			LOG_WARNING("%s, the buffer memory has been allocated", __FUNCTION__);
			return;
		}
		// ·ÖÅä¿Õ¼ä
		vk::MemoryRequirements vkMemRequire = device.getBufferMemoryRequirements(nativeBuffer);

		vk::DeviceMemory memory = AllocateDeviceMemory(vkMemRequire, memoryPorp);
		m_mapVkMemory[bufferID] = memory;
		device.bindBufferMemory(nativeBuffer, memory, 0);
	}
	void DeviceMemoryManager::AllocateImageMemory(DeviceImage& image, vk::MemoryPropertyFlags memoryPorp)
	{
		ImageID imageID = image.GetImageID();
		vk::Image nativeImage = image.GetNativeDeviceImage();
		vk::Device& device = vkContext::GetVkDevice();

		if (m_mapVkImageMemory.contains(imageID)) {
			LOG_WARNING("%s, the image memory has been allocated", __FUNCTION__);
			return;
		}

		vk::MemoryRequirements vkMemRequire = device.getImageMemoryRequirements(nativeImage);
		vk::DeviceMemory memory = AllocateDeviceMemory(vkMemRequire, memoryPorp);
		m_mapVkImageMemory[imageID] = memory;
		device.bindImageMemory(nativeImage, memory, 0);

	}
	void DeviceMemoryManager::AllocateMemory(VertexBuffer* pVertexBuffer)
	{
		GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pVertexBuffer), vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	void DeviceMemoryManager::AllocateMemory(StagingBuffer* pStagingBuffer)
	{
		GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pStagingBuffer), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	}
	void DeviceMemoryManager::AllocateMemory(IndexBuffer* pIndexBuffer)
	{
		GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pIndexBuffer), vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	void DeviceMemoryManager::AllocateMemory(ConstBuffer* pConstBuffer)
	{
		GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pConstBuffer), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	}
	void DeviceMemoryManager::AsignMemory(StagingBuffer* stagingBuffer, size_t nSize, void* pData)
	{
		auto iter = GetInstance().m_mapVkMemory.find(stagingBuffer->GetBufferID());
		if (iter == GetInstance().m_mapVkMemory.end())
		{
			LOG_WARNING("%s, the Buffer did not exist", __FUNCTION__);
		}

		RENDERER_ASSERT(stagingBuffer->Size() >= nSize, "out of bounds");

		// Ìî³ä
		vk::Device& device = vkContext::GetVkDevice();
		void* pDstData = device.mapMemory(iter->second, 0, nSize);
		memcpy(pDstData, pData, nSize);
		device.unmapMemory(iter->second);
	}
	void DeviceMemoryManager::AsignMemory(ConstBuffer* pConstBuffer, size_t nSize, void* pData)
	{
		auto iter = GetInstance().m_mapVkMemory.find(pConstBuffer->GetBufferID());
		if (iter == GetInstance().m_mapVkMemory.end())
		{
			LOG_WARNING("%s, the Buffer did not exist", __FUNCTION__);
		}

		RENDERER_ASSERT(pConstBuffer->Size() >= nSize, "out of bounds");

		// Ìî³ä
		vk::Device& device = vkContext::GetVkDevice();
		void* pDstData = device.mapMemory(iter->second, 0, nSize);
		memcpy(pDstData, pData, nSize);
		device.unmapMemory(iter->second);
	}
	void DeviceMemoryManager::FreeMemory(Buffer& buffer)
	{
		auto iter = GetInstance().m_mapVkMemory.find(buffer.GetBufferID());
		if (iter != GetInstance().m_mapVkMemory.end())
		{
			vkContext::GetVkDevice().freeMemory(iter->second);
			GetInstance().m_mapVkMemory.erase(iter);
		}
		else
		{
			LOG_WARNING("%s, the buffer did not allocate", __FUNCTION__);
		}
	}
	void DeviceMemoryManager::AllocateMemory(Image2DShaderRes* pImage)
	{
		DeviceMemoryManager& instance = GetInstance();
		instance.AllocateImageMemory(*reinterpret_cast<DeviceImage*>(pImage), vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	void DeviceMemoryManager::AsignMemory(Image2DShaderRes* pImage, size_t nSize, void* pData)
	{
		StagingBuffer* pStagingBuffer = BufferManager::CreateStagingBuffer(nSize, pData);

		vk::BufferImageCopy bic;
		bic
			.setBufferOffset(0)
			.setImageOffset(vk::Offset3D(0, 0, 0))
			.setImageExtent(vk::Extent3D(pImage->GetWidth(), pImage->GetHeight(), 1))
			;

		vk::CommandBuffer vkCmdBuffer = vkContext::BeginSingleTimeCmdBuffer();
		vkCmdBuffer.copyBufferToImage(
			pStagingBuffer->GetNativeBuffer(), 
			pImage->GetNativeDeviceImage(), 
			vk::ImageLayout::eTransferDstOptimal,
			bic
		);
		vkContext::EndSingleTimeCmdBuffer(vkCmdBuffer);


		BufferManager::DeleteBuffer(pStagingBuffer->GetBufferID());
	}
	void DeviceMemoryManager::FreeImageMemory(DeviceImage& image)
	{
		DeviceMemoryManager& instance = DeviceMemoryManager::GetInstance();
		auto iter = instance.m_mapVkImageMemory.find(image.GetImageID());
		if (iter == instance.m_mapVkImageMemory.end())
		{
			LOG_WARNING("%s, the image did not allocate", __FUNCTION__);
		}
		else
		{
			vk::Device& device = vkContext::GetVkDevice();
			device.freeMemory(iter->second);
			instance.m_mapVkImageMemory.erase(iter);
		}
	}
} // namespace LT