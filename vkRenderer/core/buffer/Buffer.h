#pragma once
#include "vkContext.h"

namespace LT {

	enum class BufferDataType {
		TypeInt8,
		TypeUint8,
		TypeInt16,
		TypeUint16,
		TypeInt32,
		TypeUint32,
		TypeInt64,
		TypeUInt64,
		TypeFloat16,
		TypeFloat32,
		TypeFloat64
	};

	static size_t BufferDataType2Size(BufferDataType eBufferDataType) {
		switch (eBufferDataType)
		{
		case LT::BufferDataType::TypeInt8:
			return 1u;
			break;
		case LT::BufferDataType::TypeUint8:
			return 1u;
			break;
		case LT::BufferDataType::TypeInt16:
			return 2u;
			break;
		case LT::BufferDataType::TypeUint16:
			return 2u;
			break;
		case LT::BufferDataType::TypeInt32:
			return 4u;
			break;
		case LT::BufferDataType::TypeUint32:
			return 4u;
			break;
		case LT::BufferDataType::TypeInt64:
			return 8u;
			break;
		case LT::BufferDataType::TypeUInt64:
			return 8u;
			break;
		case LT::BufferDataType::TypeFloat16:
			return 2u;
			break;
		case LT::BufferDataType::TypeFloat32:
			return 4u;
			break;
		case LT::BufferDataType::TypeFloat64:
			return 8u;
			break;
		default:
			return 0u;
			break;
		}
		return 0u;
	}

	class Buffer {
	protected:
		uint8_t* m_pBuffer;
		size_t m_nSize;

		vk::Buffer m_vkBuffer;
	public:
		Buffer();
		Buffer(size_t nSize, void * pData = nullptr);
		virtual ~Buffer();

		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other) noexcept;

		Buffer& operator = (const Buffer& other) = delete;
		Buffer& operator = (Buffer&& other)noexcept;

		size_t Size() const;
		uint8_t* Data();
		const uint8_t* Data() const;

		virtual void Release();

		vk::Buffer GetNativeBuffer();

		virtual void UpdateDataToGPU() = 0;
		virtual void ReleaseDeviceMemory() = 0;
	};
}