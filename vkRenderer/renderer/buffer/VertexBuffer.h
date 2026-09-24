#pragma once
#include "Buffer.h"
#include "IBindable.h"
#include "BufferDst.h"

namespace LT {

	class StagingBuffer;

	class VertexBuffer : public BufferDst, public IBindable{
		friend class BufferManager;
	protected:
		uint64_t m_nVertexCount;

		VertexBuffer(BufferID id);
		VertexBuffer(BufferID id, size_t nSize, const void* pData, uint64_t vertexCount);

		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer(const VertexBuffer&) = delete;

		VertexBuffer& operator = (const VertexBuffer&) = delete;
		VertexBuffer& operator = (VertexBuffer&&) = delete;

		~VertexBuffer();
	public:




		/// <summary>
		/// 
		/// </summary>
		/// <param name="nTarget"> 目前没有任何含义 </param>
		void Bind(BindTarget nTarget) override;
		void Unbind() override;

		void UpdateDataToGPU() override;

		uint64_t GetVertexCount() const;
	};

}// namespace LT