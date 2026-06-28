#include "vkRendererCommon.h"
#include "vkContext.h"

#include "ConstBuffer.h"

#include "DeviceMemoryManager.h"

namespace LT {
	ConstBuffer::ConstBuffer(BufferID id)
		: Buffer(id)
		, m_pMapped(nullptr)
	{
	}
	ConstBuffer::ConstBuffer(BufferID id, size_t nSize, void* pBuffer)
		: Buffer(id, nSize, pBuffer)
		, m_pMapped(nullptr)
	{
		if (nSize && pBuffer)
		{
			UpdateDataToGPU();
		}

	}
	ConstBuffer::~ConstBuffer()
	{
		DeviceMemoryManager::GetInstance().UnmapMemory(*this);
	}
	void ConstBuffer::UpdateDataToGPU()
	{
		vk::Device& device = vkContext::GetVkDevice();
		// 创建Buffer对象
		if (!m_vkBuffer)
		{
			vk::BufferCreateInfo bci;
			bci.setSize(m_nSize)
				.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
				.setSharingMode(vk::SharingMode::eExclusive)
				;
			m_vkBuffer = device.createBuffer(bci);

			// 分配空间
			DeviceMemoryManager::AllocateMemory(this);

			m_pMapped = DeviceMemoryManager::GetInstance().MapMemory(*reinterpret_cast<Buffer*>(this));
		}

		// 填充
		ConstBuffer::UpdateConstBuffer();
	}
	void ConstBuffer::UpdateConstBuffer()
	{
		// DeviceMemoryManager::AsignMemory(this, m_nSize, m_pBuffer);
		memcpy(m_pMapped, m_pBuffer, m_nSize);
	}
	void ConstBuffer::UpdateConstBuffer(const void* pData)
	{
		memcpy(m_pBuffer, pData, m_nSize);
		UpdateConstBuffer();
	}
	void ConstBuffer::Bind(BindTarget nTarget)
	{
		// TODO
	}
	void ConstBuffer::Unbind()
	{
		// TODO
	}
}