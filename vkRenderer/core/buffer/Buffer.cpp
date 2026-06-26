#include "vkRendererCommon.h"
#include "Buffer.h"
#include "DeviceMemoryManager.h"

namespace LT
{
	Buffer::Buffer(BufferID id)
		:m_nID(id)
		,m_pBuffer(nullptr)
		,m_nSize(0)
	{

	}
	Buffer::Buffer(BufferID id, size_t nSize, void* pData)
		:m_nID(id)
		,m_nSize(nSize)
	{
		if (nSize)
		{
			m_pBuffer = new uint8_t[nSize];
			if (pData) {
				memcpy(m_pBuffer, pData, nSize);
			}
		}
		else
		{
			m_pBuffer = nullptr;
			return;
		}
	}

	Buffer::~Buffer() {
		Release();
	}


	void Buffer::Release() {
		if (m_pBuffer)
		{
			delete[] m_pBuffer;
			m_pBuffer = nullptr;
		}
		m_nSize = 0;

		vk::Device& device = vkContext::GetVkDevice();
		device.destroyBuffer(m_vkBuffer);
		ReleaseDeviceMemory();
	}

	vk::Buffer Buffer::GetNativeBuffer()
	{
		return m_vkBuffer;
	}

	BufferID Buffer::GetBufferID() const
	{
		return m_nID;
	}

	void Buffer::ReleaseDeviceMemory()
	{
		DeviceMemoryManager::FreeMemory(*this);
	}

	size_t Buffer::Size() const{
		return m_nSize;
	}
	
	const uint8_t* Buffer::Data() const {
		return m_pBuffer;
	}

	uint8_t* Buffer::Data() {
		return m_pBuffer;
	}

}// namepace LT