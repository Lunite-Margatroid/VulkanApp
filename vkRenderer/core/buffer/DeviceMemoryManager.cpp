#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DeviceMemoryManager.h"

namespace LT {

	DeviceMemoryManager* DeviceMemoryManager::s_pDeviceMemoryManagerInstance = nullptr;

	DeviceMemoryManager& DeviceMemoryManager::GetInstance() {
		RENDERER_ASSERT(s_pDeviceMemoryManagerInstance,"DeviceMemoryManager did not init.");
		return *s_pDeviceMemoryManagerInstance;
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

	void DeviceMemoryManager::AllocateMemory(Buffer& buffer, vk::MemoryPropertyFlags memoryPorp)
	{
		BufferID bufferID = buffer.GetBufferID();
		vk::Buffer nativeBuffer = buffer.GetNativeBuffer();

		if (m_mapVkMemory.contains(bufferID)) {
			return;
		}


		vk::Device& device = vkContext::GetVkDevice();
		vk::DeviceMemory memory;


		// ∑÷≈‰ø’º‰
		vk::MemoryRequirements vkMemRequire = device.getBufferMemoryRequirements(nativeBuffer);

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
		m_mapVkMemory[bufferID] = memory;
		device.bindBufferMemory(nativeBuffer, memory, 0);
	}
	void DeviceMemoryManager::AllocateMemory(VertexBuffer* pVertexBuffer)
	{
		GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pVertexBuffer), vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	void DeviceMemoryManager::AllocateMemory(StagingBuffer* pStagingBuffer)
	{
		GetInstance().AllocateMemory(*reinterpret_cast<Buffer*>(pStagingBuffer), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	}
	void DeviceMemoryManager::AsignMemory(StagingBuffer* stagingBuffer, size_t nSize, void* pData)
	{
		auto iter = GetInstance().m_mapVkMemory.find(stagingBuffer->GetBufferID());
		if (iter == GetInstance().m_mapVkMemory.end())
		{
			LOG_WARNING("%s, the Buffer did not exist", __FUNCTION__);
		}

		RENDERER_ASSERT(stagingBuffer->Size() >= nSize, "out of bounds");

		// ÃÓ≥‰
		vk::Device& device =  vkContext::GetVkDevice();
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
} // namespace LT