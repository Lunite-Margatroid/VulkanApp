#pragma once
#include "Buffer.h"
#include "IBindable.h"

namespace LT {
	class ConstBuffer : public Buffer , public IBindable{
		friend class BufferManager;

	protected:
		// 通过Device::mapMemory获取的指针
		void* m_pMapped;

	protected:
		ConstBuffer(BufferID id);
		ConstBuffer(BufferID id, size_t nSize, void* pBuffer);

		ConstBuffer(const ConstBuffer&) = delete;
		ConstBuffer(ConstBuffer&&) = delete;

		ConstBuffer& operator = (const ConstBuffer&) = delete;
		ConstBuffer& operator = (ConstBuffer&&) = delete;
		
		~ConstBuffer();

	public:

		void UpdateDataToGPU() override;

		void UpdateConstBuffer();
		void UpdateConstBuffer(const void* pData);
	public:
		// IBindable

		void Bind(BindTarget nTarget) override;
		void Unbind() override;
	};


} // namespace LT
