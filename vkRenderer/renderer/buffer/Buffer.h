#pragma once
#include "vkContext.h"

namespace LT {

	using BufferID = int64_t;
	constexpr int64_t INVALID_BUFFER_ID = -1;

	/// <summary>
	/// 缓冲对象的基类
	/// </summary>
	class Buffer {
		friend class BufferManager;

	protected:
		const BufferID m_nID;
		uint8_t* m_pBuffer; 
		size_t m_nSize;
		vk::Buffer m_vkBuffer;

	protected:
		Buffer(BufferID id);
		Buffer(BufferID id, size_t nSize, const void* pData);
		virtual ~Buffer();

		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other) = delete;
		Buffer& operator = (const Buffer& other) = delete;
		Buffer& operator = (Buffer&& other) = delete;
	public:

		size_t Size() const;
		uint8_t* Data();
		const uint8_t* Data() const;

		virtual void Release();

		vk::Buffer GetNativeBuffer();
		BufferID GetBufferID() const;

		virtual void UpdateDataToGPU() = 0;
		virtual void ReleaseDeviceMemory();
	};
}