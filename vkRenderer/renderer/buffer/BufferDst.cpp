#include "vkRendererCommon.h"
#include "vkContext.h"
#include "BufferDst.h"
#include "StagingBuffer.h"
namespace LT {
	BufferDst::BufferDst(BufferID id)
		:Buffer(id)
	{
	}

	BufferDst::BufferDst(BufferID id, size_t nSize, void* pData)
		: Buffer(id, nSize, pData)
	{
	}


	void BufferDst::DeviceCopy(StagingBuffer* pStagingBuffer, const uint64_t nSize, const uint64_t nSrcOffset/* = 0u*/, const uint64_t nDstOffset/* = 0u*/)
	{

		size_t nSrcValidSize = pStagingBuffer->Size() > nSrcOffset ? (pStagingBuffer->Size() - nSrcOffset) : 0;
		size_t nDstValidSize = m_nSize > nDstOffset ? (m_nSize - nDstOffset) : 0;
		if (nSrcValidSize != nSize || nDstValidSize != nSize || nSrcValidSize == 0 || nDstValidSize == 0)
		{
			LOG_WARNING("Device Buffer Copy Faild.");
			return;
		}

		vk::CommandBuffer vkCmdBuffer = vkContext::BeginSingleTimeCmdBuffer();
		vkCmdBuffer.copyBuffer(pStagingBuffer->GetNativeBuffer(), m_vkBuffer, vk::BufferCopy(nSrcOffset, nDstOffset, m_nSize));
		vkContext::EndSingleTimeCmdBuffer(vkCmdBuffer);
	}

} // namespace LT