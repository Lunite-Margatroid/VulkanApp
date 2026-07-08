#include "vkRendererCommon.h"
#include "vkContext.h"
#include "StagingBuffer.h"

#include "BufferManager.h"
#include "DeviceMemoryManager.h"

namespace LT {
	StagingBuffer::StagingBuffer(BufferID id)
		:Buffer(id)
	{}
	StagingBuffer::StagingBuffer(BufferID id, size_t nSize, const void* pData)
	:Buffer(id, nSize, pData)
	{
		if (m_nSize && m_pBuffer)
		{
			UpdateDataToGPU();
		}
	}

	StagingBuffer::~StagingBuffer()
	{

	}

	void StagingBuffer::UpdateDataToGPU()
	{
		vk::Device& device = vkContext::GetVkDevice();
		// 创建Buffer对象
		if (!m_vkBuffer)
		{
			vk::BufferCreateInfo bci;
			bci.setSize(m_nSize)
				.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
				.setSharingMode(vk::SharingMode::eExclusive)
				;
			m_vkBuffer = device.createBuffer(bci);

			// 分配空间
			DeviceMemoryManager::AllocateMemory(this);
		}

		// 填充
		DeviceMemoryManager::AsignMemory(this, m_nSize, m_pBuffer);
	}

} // namespace LT
