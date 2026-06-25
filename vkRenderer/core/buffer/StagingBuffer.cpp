#include "vkRendererCommon.h"
#include "vkContext.h"
#include "StagingBuffer.h"

namespace LT {
	StagingBuffer::StagingBuffer() {}
	StagingBuffer::StagingBuffer(size_t nSize, void* pData)
	:Buffer(nSize, pData)
	{
		if (m_nSize && m_pBuffer)
		{
			UpdateDataToGPU();
		}
	}

	StagingBuffer::~StagingBuffer()
	{
		ReleaseDeviceMemory();
	}

	void StagingBuffer::UpdateDataToGPU()
	{
		vk::Device& device = vkContext::GetVkDevice();
		device.destroyBuffer(m_vkBuffer);
		ReleaseDeviceMemory();


		// 创建Buffer对象
		vk::BufferCreateInfo bci;
		bci.setSize(m_nSize)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive)
			;
		m_vkBuffer = device.createBuffer(bci);

		// 分配空间
		vk::MemoryRequirements vkMemRequire = device.getBufferMemoryRequirements(m_vkBuffer);

		vk::PhysicalDevice& phyDevice = vkContext::GetPhysicalDevice();
		vk::PhysicalDeviceMemoryProperties vkMemProp = phyDevice.getMemoryProperties();

		for (uint32_t i = 0; i < vkMemProp.memoryTypeCount; i++) {
			if (vkMemRequire.memoryTypeBits & (1 << i)) {
				if ((vkMemProp.memoryTypes[i].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
					== (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) {

					vk::MemoryAllocateInfo mai;
					mai.setAllocationSize(vkMemRequire.size)
						.setMemoryTypeIndex(i)
						;
					m_vkMemory = device.allocateMemory(mai);
					break;
				}
			}

		}
		RENDERER_ASSERT(m_vkMemory, "Memory Allocation Failed.");

		device.bindBufferMemory(m_vkBuffer, m_vkMemory, 0);

		void* pData = device.mapMemory(m_vkMemory, 0, m_nSize);
		memcpy(pData, m_pBuffer, m_nSize);
		device.unmapMemory(m_vkMemory);
	}

	void StagingBuffer::ReleaseDeviceMemory()
	{
		vk::Device& device = vkContext::GetVkDevice();
		device.freeMemory(m_vkMemory);
	}

} // namespace LT
