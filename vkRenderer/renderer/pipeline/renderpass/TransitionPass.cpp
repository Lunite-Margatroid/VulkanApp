#include "vkRendererCommon.h"
#include "vkContext.h"
#include "TransitionPass.hpp"

namespace LT{

	void TransitionPass::RecordCommand(const RecordTransitionInfo& sInOutRecordInfo)
	{
		vk::CommandBufferBeginInfo cbbi = {};
		sInOutRecordInfo.pCmdBuffer->Begin(cbbi);

		// 颜色缓冲
		for (RecordTransitionInfo::TransitionType sTransitionType : sInOutRecordInfo.vecColorAttachment) {
			RecordTransitionImageLayout(GenTransitionInfo(
				sTransitionType.eSrc,
				sTransitionType.eDst,
				sTransitionType.nImageID,
				sInOutRecordInfo.pCmdBuffer
			));
		}
		// 深度缓冲
		for (RecordTransitionInfo::TransitionType sTransitionType : sInOutRecordInfo.vecDepthAttachment) {
			RecordTransitionImageLayout(GenTransitionInfo(
				sTransitionType.eSrc,
				sTransitionType.eDst,
				sTransitionType.nImageID,
				sInOutRecordInfo.pCmdBuffer
			));
		}
		sInOutRecordInfo.pCmdBuffer->End();
	}
	void TransitionPass::Submit(TransitionSubmitInfo& sInOutSubmitInfo)
	{
		vk::Device& device = vkContext::GetVkDevice();
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(sInOutSubmitInfo.nFlightFrameIndex);

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
	TransitionImageLayoutInfo TransitionPass::GenTransitionInfo(PipelineIOType eSrc, PipelineIOType eDst, ImageID nImageID, CommandBufferWrapper* pCmdBuffer)
	{
		TransitionImageLayoutInfo sTransitionInfo;
		sTransitionInfo.pCmdBuffer = pCmdBuffer;

		if (eSrc == PipelineIOType::eUndefined && eDst == PipelineIOType::eColorAttachment)
		{
			sTransitionInfo.srcAccessFlag = vk::AccessFlagBits2::eNone;
			sTransitionInfo.dstAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
			sTransitionInfo.srcStageMask = vk::PipelineStageFlagBits2::eNone;
			sTransitionInfo.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
			sTransitionInfo.eOldLayout = vk::ImageLayout::eUndefined;
			sTransitionInfo.eNewLayout = vk::ImageLayout::eColorAttachmentOptimal;
			sTransitionInfo.eImageAspect = vk::ImageAspectFlagBits::eColor;

		}
		else if (eSrc == PipelineIOType::eColorAttachment && eDst == PipelineIOType::eColorPresent)
		{
			sTransitionInfo.srcAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
			sTransitionInfo.dstAccessFlag = vk::AccessFlagBits2::eNone;
			sTransitionInfo.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
			sTransitionInfo.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
			sTransitionInfo.eOldLayout = vk::ImageLayout::eColorAttachmentOptimal;
			sTransitionInfo.eNewLayout = vk::ImageLayout::ePresentSrcKHR;
			sTransitionInfo.eImageAspect = vk::ImageAspectFlagBits::eColor;
		}
		else if (eSrc == PipelineIOType::eColorPresent && eDst == PipelineIOType::eColorAttachment)
		{
			sTransitionInfo.srcAccessFlag = vk::AccessFlagBits2::eNone;
			sTransitionInfo.dstAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
			sTransitionInfo.srcStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
			sTransitionInfo.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
			sTransitionInfo.eOldLayout = vk::ImageLayout::ePresentSrcKHR;
			sTransitionInfo.eNewLayout = vk::ImageLayout::eColorAttachmentOptimal;
			sTransitionInfo.eImageAspect = vk::ImageAspectFlagBits::eColor;
		}
		else if (eSrc == PipelineIOType::eUndefined && eDst == PipelineIOType::eDepthAttachment)
		{
			sTransitionInfo.srcAccessFlag = vk::AccessFlagBits2::eNone;
			sTransitionInfo.dstAccessFlag = vk::AccessFlagBits2::eColorAttachmentRead;
			sTransitionInfo.srcStageMask = vk::PipelineStageFlagBits2::eNone;
			sTransitionInfo.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			sTransitionInfo.eOldLayout = vk::ImageLayout::eUndefined;
			sTransitionInfo.eNewLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			sTransitionInfo.eImageAspect = vk::ImageAspectFlagBits::eDepth;
		}
		else
		{
			RENDERER_ASSERT(false, "The Transition is not defined.");
		}

		return sTransitionInfo;
	}
} // namespace LT