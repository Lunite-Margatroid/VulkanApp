#include "vkRendererCommon.h"
#include "vkContext.h"
#include "TransitionPass.hpp"

namespace LT{

	void TransitionPass::RecordCommand(const RecordTransitionInfo& sInOutRecordInfo)
	{
		vk::CommandBufferBeginInfo cbbi = {};
		sInOutRecordInfo.pCmdBuffer->Begin(cbbi);

		// 颜色缓冲
		for (TransitionImageLayoutInfo sTransition : sInOutRecordInfo.vecColorAttachment) {
			sTransition.pCmdBuffer = sInOutRecordInfo.pCmdBuffer;
			RecordTransitionImageLayout(sTransition);
		}
		// 深度缓冲
		for (TransitionImageLayoutInfo sTransition : sInOutRecordInfo.vecDepthAttachment) {
			sTransition.pCmdBuffer = sInOutRecordInfo.pCmdBuffer;
			RecordTransitionImageLayout(sTransition);
		}
		sInOutRecordInfo.pCmdBuffer->End();
	}
	void TransitionPass::Submit(TransitionSubmitInfo& sInOutSubmitInfo)
	{
		vk::Device& device = vkContext::GetVkDevice();
		CommandBufferWrapper cmdBuffer = vkContext::GetCmdBuffer(sInOutSubmitInfo.nFlightFrameIndex);

		CmdSubmitInfo sCmdSubmitInfo;
		// 需要发射的信号量
		if (sInOutSubmitInfo.vecSemToSignal.size() > 0)
		{
			sCmdSubmitInfo.vecSemToSignal = sInOutSubmitInfo.vecSemToSignal;
		}

		// 需要等待的信号量
		if (sInOutSubmitInfo.vecSemToWait.size() > 0)
		{
			sCmdSubmitInfo.vecSemToWait = sInOutSubmitInfo.vecSemToWait;
			sCmdSubmitInfo.vecSemWaitMasks = sInOutSubmitInfo.vecSemWaitMasks;
		}


		// 本次Submit的Fence
		sInOutSubmitInfo.refFence = cmdBuffer.Submit(sCmdSubmitInfo);
	}
} // namespace LT