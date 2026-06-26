#pragma once
#include "BufferDst.h"
#include "IBindable.h"

namespace LT {
	class IndexBuffer : public BufferDst , public IBindable{
		friend class BufferManager;

	protected:
		uint64_t m_nIndexCount;

	protected:
		IndexBuffer(BufferID id);
		IndexBuffer(BufferID id, size_t nSize, void* pData, uint64_t nIndexCount);

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer(IndexBuffer&&) = delete;

		IndexBuffer& operator = (const IndexBuffer&) = delete;
		IndexBuffer& operator = (IndexBuffer&&) = delete;
	public:
		void Bind(BindTarget nTarget) override;
		void Unbind() override;

		void UpdateDataToGPU() override;

		uint64_t GetIndexCount() const;
	};
} // namespace LT
