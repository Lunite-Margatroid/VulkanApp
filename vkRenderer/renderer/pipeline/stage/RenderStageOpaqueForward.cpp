// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"

namespace LT {
	RenderStageOpaqueForward::RenderStageOpaqueForward()
	{
		// 转换ColorAttachement
		m_sTransitionColorToDraw.srcAccessFlag = vk::AccessFlagBits2::eNone;
		m_sTransitionColorToDraw.dstAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
		m_sTransitionColorToDraw.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		m_sTransitionColorToDraw.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		m_sTransitionColorToDraw.eOldLayout = vk::ImageLayout::eUndefined;
		m_sTransitionColorToDraw.eNewLayout = vk::ImageLayout::eColorAttachmentOptimal;
		m_sTransitionColorToDraw.eImageAspect = vk::ImageAspectFlagBits::eColor;
		// 转换为DepthAttachment
		m_sTransitionDepthToDraw.srcAccessFlag = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
		m_sTransitionDepthToDraw.dstAccessFlag = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
		m_sTransitionDepthToDraw.srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
		m_sTransitionDepthToDraw.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
		m_sTransitionDepthToDraw.eOldLayout = vk::ImageLayout::eUndefined;
		m_sTransitionDepthToDraw.eNewLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		m_sTransitionDepthToDraw.eImageAspect = vk::ImageAspectFlagBits::eDepth;
		// 转换为Color Present
		m_sTransitionColorToPresent.eOldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		m_sTransitionColorToPresent.eNewLayout = vk::ImageLayout::ePresentSrcKHR;
		m_sTransitionColorToPresent.srcAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
		m_sTransitionColorToPresent.dstAccessFlag = vk::AccessFlagBits2::eNone;
		m_sTransitionColorToPresent.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		m_sTransitionColorToPresent.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
		m_sTransitionColorToPresent.eImageAspect = vk::ImageAspectFlagBits::eColor;

		// 创建Semaphore
		vk::Device& device = vkContext::GetVkDevice();
		for (int i = 0; i < m_arrSemTransitionBegin.size(); ++i)
		{
			m_arrSemTransitionBegin[i] = device.createSemaphore(vk::SemaphoreCreateInfo{});
		}

		for (int i = 0; i < m_arrSemTransitionEnd.size(); ++i)
		{
			m_arrSemTransitionEnd[i] = device.createSemaphore(vk::SemaphoreCreateInfo{});
		}
	}
	RenderStageOpaqueForward::~RenderStageOpaqueForward()
	{
		// 销毁Semaphore
		vk::Device& device = vkContext::GetVkDevice();
		for (int i = 0; i < m_arrSemTransitionBegin.size(); ++i)
		{
			if (m_arrSemTransitionBegin[i])
			{
				device.destroySemaphore(m_arrSemTransitionBegin[i]);
			}
		}

		for (int i = 0; i < m_arrSemTransitionEnd.size(); ++i)
		{
			if (m_arrSemTransitionEnd[i])
			{
				device.destroySemaphore(m_arrSemTransitionEnd[i]);
			}
		}
	}
	void RenderStageOpaqueForward::Execute(StageExecuteInfo& sInOutExecuteInfo)
	{
		if (sInOutExecuteInfo.vecRenderEntity.size() <= 0)
			return;
		// ----------------------------- Stage Begin------------------------
		// Barrier
		// 进行Transition
		StageBeginInfo sStageBeginInfo;
		sStageBeginInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
		sStageBeginInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
		sStageBeginInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		sStageBeginInfo.vecSemWait = sInOutExecuteInfo.vecSemWait;
		sStageBeginInfo.vecSemWaitMask = sInOutExecuteInfo.vecSemWaitMask;
		sStageBeginInfo.vecSemSignal.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
		StageBegin(sStageBeginInfo);

		//sStageBeginInfo.refFence.WaitFence();

		// -------------------------- Draw -------------------------------
		EntityDrawInfo sDrawInfo;
		sDrawInfo.nWidth = sInOutExecuteInfo.nWidth;
		sDrawInfo.nHeight = sInOutExecuteInfo.nHeight;
		sDrawInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		sDrawInfo.nDepthBuffer = sInOutExecuteInfo.nDepthBufferID;
		sDrawInfo.vecRenderTargets = sInOutExecuteInfo.vecRenderTarget;
		sDrawInfo.pRenderView = sInOutExecuteInfo.pRenderView;
		sDrawInfo.eRenderStage = GetRenderStageType();
		sDrawInfo.bClear = true;

		if (sInOutExecuteInfo.vecRenderEntity.size() >= 2)
		{
			// 第一个渲染实体
			// 等待Transition
			sDrawInfo.vecSemWait.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
			sDrawInfo.vecSemWaitMasks.push_back(vk::PipelineStageFlagBits::eAllCommands);

			sInOutExecuteInfo.vecRenderEntity.front()->Draw(sDrawInfo);

			sDrawInfo.vecSemWait.clear();
			sDrawInfo.vecSemWaitMasks.clear();

			sDrawInfo.bClear = false;
			// 第2到n个渲染实体
			for (int i = 1; i < sInOutExecuteInfo.vecRenderEntity.size() - 1; i++)
			{
				sInOutExecuteInfo.vecRenderEntity[i]->Draw(sDrawInfo);
			}
			// 最后一个渲染实体

			sDrawInfo.vecSemSignal.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);

			sInOutExecuteInfo.vecRenderEntity.back()->Draw(sDrawInfo);
		}
		else if(sInOutExecuteInfo.vecRenderEntity.size() > 0)
		{
			// 等待Transition
			sDrawInfo.vecSemWait.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
			sDrawInfo.vecSemWaitMasks.push_back(vk::PipelineStageFlagBits::eAllCommands);

			sDrawInfo.vecSemSignal.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);

			sInOutExecuteInfo.vecRenderEntity.front()->Draw(sDrawInfo);
		}
		//sDrawInfo.refFence.WaitFence();

		// ---------------- Stage End -------------
		// Barrier
		StageEndInfo sStageEndInfo;
		sStageEndInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
		sStageEndInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
		sStageEndInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		sStageEndInfo.vecSemWait.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);
		sStageEndInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllGraphics);
		if (!sInOutExecuteInfo.vecSemSignal.empty())
		{
			sStageEndInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
		}
		StageEnd(sStageEndInfo);

		sInOutExecuteInfo.refFence = sStageEndInfo.refFence;
	}
	void RenderStageOpaqueForward::StageBegin(StageBeginInfo& sInOutBeginInfo)
	{
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(sInOutBeginInfo.nFlightFrameIndex);

		RecordTransitionInfo sRecordInfo;
		sRecordInfo.pCmdBuffer = &cmdBuffer;
		// Color Attachment
		for (ImageID nImage : sInOutBeginInfo.vecRenderTarget) {
			TransitionImageLayoutInfo sTransInfo = m_sTransitionColorToDraw;
			sTransInfo.nImageID = nImage;
			sTransInfo.pCmdBuffer = &cmdBuffer;
			sRecordInfo.vecColorAttachment.push_back(sTransInfo);
		}
		// Depth Attachment
		TransitionImageLayoutInfo sDepthTransInfo = m_sTransitionDepthToDraw;
		sDepthTransInfo.nImageID = sInOutBeginInfo.nDepthBufferID;
		sDepthTransInfo.pCmdBuffer = &cmdBuffer;
		sRecordInfo.vecDepthAttachment.push_back(sDepthTransInfo);

		m_passTransition.RecordCommand(sRecordInfo);


		TransitionSubmitInfo sSubmitInfo;
		sSubmitInfo.nFlightFrameIndex = sInOutBeginInfo.nFlightFrameIndex;
		sSubmitInfo.vecSemToSignal = sInOutBeginInfo.vecSemSignal;
		sSubmitInfo.vecSemToWait = sInOutBeginInfo.vecSemWait;
		sSubmitInfo.vecSemWaitMasks = sInOutBeginInfo.vecSemWaitMask;

		m_passTransition.Submit(sSubmitInfo);

		sInOutBeginInfo.refFence = sSubmitInfo.refFence;
	}

	void RenderStageOpaqueForward::StageEnd(StageEndInfo& sInOutEndInfo) {
		RecordTransitionInfo sRecordInfo;
		sRecordInfo.pCmdBuffer = &vkContext::GetCmdBuffer(sInOutEndInfo.nFlightFrameIndex);
		// Color Attachment
		for (ImageID nImage : sInOutEndInfo.vecRenderTarget) {
			TransitionImageLayoutInfo sTransInfo = m_sTransitionColorToPresent;
			sTransInfo.nImageID = nImage;
			sRecordInfo.vecColorAttachment.push_back(sTransInfo);
		}

		m_passTransition.RecordCommand(sRecordInfo);


		TransitionSubmitInfo sSubmitInfo;
		sSubmitInfo.nFlightFrameIndex = sInOutEndInfo.nFlightFrameIndex;
		sSubmitInfo.vecSemToSignal = sInOutEndInfo.vecSemSignal;
		sSubmitInfo.vecSemToWait = sInOutEndInfo.vecSemWait;
		sSubmitInfo.vecSemWaitMasks = sInOutEndInfo.vecSemWaitMask;

		m_passTransition.Submit(sSubmitInfo);

		sInOutEndInfo.refFence = sSubmitInfo.refFence;
	}
} // namespace LT