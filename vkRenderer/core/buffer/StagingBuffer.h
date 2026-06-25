#pragma once
#include "Buffer.h"

namespace LT {
	class StagingBuffer : public Buffer {

	protected:
		vk::DeviceMemory m_vkMemory;

	public:
		StagingBuffer();
		StagingBuffer(size_t nSize, void* pData);
		~StagingBuffer();

		virtual void UpdateDataToGPU() override;
		virtual void ReleaseDeviceMemory() override;
	};
	
} // namespace LT