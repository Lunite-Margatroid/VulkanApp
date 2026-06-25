#include "vkRendererCommon.h"
#include "Buffer.h"

namespace LT
{
	Buffer::Buffer()
		:m_pBuffer(nullptr)
		, m_nSize(0)
	{

	}
	Buffer::Buffer(size_t nSize, void* pData)
		:m_nSize(nSize)
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


	Buffer::Buffer(Buffer&& other) noexcept
	{
		m_nSize = other.m_nSize;
		m_pBuffer = other.m_pBuffer;

		other.m_nSize = 0;
		other.m_pBuffer = nullptr;

		m_vkBuffer = other.m_vkBuffer;
		other.m_vkBuffer = vk::Buffer();
	}

	Buffer&  Buffer::operator =(Buffer&& other)noexcept {
		if (this != &other)
		{
			std::swap(m_pBuffer, other.m_pBuffer);
			std::swap(m_nSize, other.m_nSize);
			std::swap(m_vkBuffer, other.m_vkBuffer);
		}
		return *this;
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
	}

	vk::Buffer Buffer::GetNativeBuffer()
	{
		return m_vkBuffer;
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