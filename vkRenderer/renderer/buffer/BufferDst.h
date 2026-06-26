#pragma once
#include "Buffer.h"

namespace LT {
	class StagingBuffer;

	/// <summary>
	/// 纯虚类。其子类必须满足vk::BufferUsageFlagBits::eTransferDst
	/// </summary>
	class BufferDst : public Buffer{
	protected:
		BufferDst(BufferID id, size_t nSize, void* pData);
		BufferDst(BufferID id);

	public:
		/// <summary>
		/// 把pStagingBuffer的Device数据拷贝到当前对象
		/// </summary>
		/// <param name="pStagingBuffer"> 源数据 </param>
		/// <param name="nSize"> 拷贝数据的尺寸 </param>
		/// <param name="nSrcOffset"> 源数据的offset </param>
		/// <param name="nDstOffset"> 目标数据的offset </param>
		void DeviceCopy(StagingBuffer* pStagingBuffer, const uint64_t nSize, const uint64_t nSrcOffset = 0u, const uint64_t nDstOffset = 0u);
	};

}// namespace LT