#include "vkRendererCommon.h"
#include "vkContext.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"

#include "BufferManager.h"
#include "DeviceMemoryManager.h"

namespace LT {


	VertexBuffer::VertexBuffer(BufferID id)
		:BufferDst(id),
		m_nVertexCount(0)
	{

	}

	VertexBuffer::VertexBuffer(BufferID id, size_t nSize, const void* pData, uint64_t vertexCount)
		:BufferDst(id, nSize, pData),
		m_nVertexCount(vertexCount)
	{
		if (nSize && pData) {
			UpdateDataToGPU();
		}
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Bind(BindTarget nTarget)
	{
		// TODO
	}

	void VertexBuffer::Unbind()
	{
		// TODO
	}

	void VertexBuffer::UpdateDataToGPU()
	{
		vk::Device& device = vkContext::GetVkDevice();

		if (!m_vkBuffer)
		{
			// 创建Buffer对象
			vk::BufferCreateInfo bci;
			bci.setSize(m_nSize)
				.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eExclusive)
				;
			VmaAllocationCreateInfo vaci = {};
			vaci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

			m_vkBuffer = DeviceMemoryManager::CreateBuffer(m_nID, bci, vaci);

			// m_vkBuffer = device.createBuffer(bci);

			// 分配空间
			// DeviceMemoryManager::AllocateMemory(this);
		}
		// 填充
		StagingBuffer* stagingBuffer = BufferManager::CreateStagingBuffer(m_nSize, m_pBuffer);
		DeviceCopy(stagingBuffer, m_nSize);
		BufferManager::DeleteBuffer(stagingBuffer);
	}

	uint64_t VertexBuffer::GetVertexCount() const
	{
		return m_nVertexCount;
	}
} //namespace LT
