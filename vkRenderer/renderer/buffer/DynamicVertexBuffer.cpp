#include "vkRendererCommon.h"
#include "vkContext.h"

#include "DynamicVertexBuffer.hpp"
#include "BufferManager.h"
#include "DeviceMemoryManager.h"

namespace LT
{
	DynamicVertexBuffer::~DynamicVertexBuffer()
	{}
	void DynamicVertexBuffer::UpdateDataToGPU() {
		vk::Device& device = vkContext::GetVkDevice();

		if (!m_vkBuffer)
		{
			vk::BufferCreateInfo bci = {};
			bci
				.setSize(m_nSize)
				.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eExclusive)
				;

			VmaAllocationCreateInfo vaci = {};
			vaci.usage = VMA_MEMORY_USAGE_AUTO;
			vaci.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			vaci.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			vaci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}

		m_pMapped = DeviceMemoryManager::GetInstance().MapMemory(*reinterpret_cast<Buffer*>(this));

		UpdateDynamicBuffer();
	}
	void DynamicVertexBuffer::UpdateDynamicBuffer()
	{
		memcpy(m_pMapped, m_pBuffer, m_nSize);
	}
	void DynamicVertexBuffer::UpdateDynamicBuffer(const void* pData)
	{
		memcpy(m_pBuffer, pData, m_nSize);
	}
	void DynamicVertexBuffer::UpdateDynamicBuffer(const void* pData, size_t nOffset, size_t nSize)
	{
		RENDERER_ASSERT(m_nSize - nOffset >= nSize, "out of bounds");
		memcpy(m_pBuffer + nOffset, pData, nSize);
	}
	DynamicVertexBuffer::DynamicVertexBuffer(BufferID id)
		: VertexBuffer(id)
		, m_pMapped(nullptr)
	{}
	DynamicVertexBuffer::DynamicVertexBuffer(BufferID id, size_t nSize, const void* pData, uint64_t vertexCount)
		: VertexBuffer(id, nSize, pData, vertexCount)
		, m_pMapped(nullptr)
	{}
}