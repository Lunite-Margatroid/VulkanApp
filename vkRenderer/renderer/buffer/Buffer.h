#pragma once
#include "vkContext.h"

namespace LT {

	enum class BufferDataType {
		TypeUnknown,
		TypeBinary,
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

	using BufferID = int64_t;

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
		Buffer(BufferID id, size_t nSize, void* pData);
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