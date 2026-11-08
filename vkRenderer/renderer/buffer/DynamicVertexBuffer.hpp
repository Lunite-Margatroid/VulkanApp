#pragma once
#include "VertexBuffer.h"

namespace LT {
	class DynamicVertexBuffer : public VertexBuffer {
		friend class BufferManager;
	protected:

		void* m_pMapped;

		DynamicVertexBuffer(BufferID id);
		DynamicVertexBuffer(BufferID id, size_t nSize, const void* pData, uint64_t vertexCount);
		DynamicVertexBuffer(DynamicVertexBuffer&&) = delete;
		DynamicVertexBuffer(const DynamicVertexBuffer&) = delete;
		DynamicVertexBuffer& operator = (const DynamicVertexBuffer&) = delete;
		DynamicVertexBuffer& operator = (DynamicVertexBuffer&&) = delete;
		~DynamicVertexBuffer();
	public:
		void UpdateDataToGPU() override;
		// 内存数据同步到GPU
		void UpdateDynamicBuffer();
		void UpdateDynamicBuffer(const void* pData);
		void UpdateDynamicBuffer(const void* pData, size_t nOffset, size_t nSize);
	};
}// namespace LT