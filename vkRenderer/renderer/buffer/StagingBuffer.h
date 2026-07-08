#pragma once
#include "Buffer.h"

namespace LT {
	class StagingBuffer : public Buffer {
		friend class BufferManager;
	protected:

	protected:
		StagingBuffer(BufferID id);
		StagingBuffer(BufferID id, size_t nSize, const void* pData);
		~StagingBuffer();
	public:


		virtual void UpdateDataToGPU() override;
	};
	
} // namespace LT