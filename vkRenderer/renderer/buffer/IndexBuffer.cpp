#include "vkRendererCommon.h"
#include "vkContext.h"
#include "IndexBuffer.h"
#include "StagingBuffer.h"
#include "DeviceMemoryManager.h"
#include "BufferManager.h"

namespace LT {
	IndexBuffer::IndexBuffer(BufferID id)
		: BufferDst(id)
		, m_nIndexCount(0)
	{
	}
	IndexBuffer::IndexBuffer(BufferID id, size_t nSize, void* pData, uint64_t nIndexCount)
		: BufferDst(id, nSize, pData)
		, m_nIndexCount(nIndexCount)
	{
		if (nSize && pData)
		{
			UpdateDataToGPU();
		}

	}
	void IndexBuffer::Bind(BindTarget nTarget)
	{
		// TODO
	}
	void IndexBuffer::Unbind()
	{
		//TODO
	}
	void IndexBuffer::UpdateDataToGPU()
	{
		vk::Device& device = vkContext::GetVkDevice();

		if (!m_vkBuffer)
		{
			// 创建Buffer对象
			vk::BufferCreateInfo bci;
			bci.setSize(m_nSize)
				.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eExclusive)
				;
			//m_vkBuffer = device.createBuffer(bci);

			//// 分配空间
			//DeviceMemoryManager::AllocateMemory(this);

			VmaAllocationCreateInfo vaci = {};
			vaci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			
			m_vkBuffer = DeviceMemoryManager::CreateBuffer(m_nID, bci, vaci);


		}
		// 填充
		StagingBuffer* stagingBuffer = BufferManager::CreateStagingBuffer(m_nSize, m_pBuffer);
		DeviceCopy(stagingBuffer, m_nSize);
		BufferManager::DeleteBuffer(stagingBuffer);
	}
	uint64_t IndexBuffer::GetIndexCount() const
	{
		return m_nIndexCount;
	}
} // namespace LT