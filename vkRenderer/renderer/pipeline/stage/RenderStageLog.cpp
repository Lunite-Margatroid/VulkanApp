#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "RenderStageLog.hpp"

#include "ImageManager.h"

#include "EntityRenderLogLine.hpp"

namespace LT {
	void LT::RenderStageLog::Execute(StageExecuteInfo& sInOutExecuteInfo)
	{
		if (sInOutExecuteInfo.mapRenderEntity.size() <= 0) return;

		const FlightFrameIndex nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(nFlightFrameIndex);

		// ----------------------------- Stage Begin------------------------
		// Barrier
		// 进行Transition
		StageBeginInfo sStageBeginInfo;
		sStageBeginInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
		sStageBeginInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
		sStageBeginInfo.nFlightFrameIndex = nFlightFrameIndex;
		sStageBeginInfo.vecSemWait = sInOutExecuteInfo.vecSemWait;
		sStageBeginInfo.vecSemWaitMask = sInOutExecuteInfo.vecSemWaitMask;
		sStageBeginInfo.vecSemSignal.push_back(m_arrSemTransitionBegin[nFlightFrameIndex]);
		StageBegin(sStageBeginInfo);

		// -------------------------- Draw ---------------------------------

		vk::CommandBufferBeginInfo cbbi = {};
		cmdBuffer.Begin(cbbi);


		vk::ClearValue colorClear = vk::ClearColorValue(0.f, 0.f, 0.f, 1.f);
		std::vector<vk::RenderingAttachmentInfo> vecColorAttachmentInfo;
		for (int i = 0; i < sInOutExecuteInfo.vecRenderTarget.size(); ++i)
		{
			vk::RenderingAttachmentInfo rai = {};
			rai
				.setImageView(ImageManager::GetInstance().GetNativeDeviceImageView(sInOutExecuteInfo.vecRenderTarget[i]))
				.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
				.setLoadOp(sInOutExecuteInfo.vecColorClear[i] ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setClearValue(colorClear)
				;
			vecColorAttachmentInfo.push_back(rai);
		}

		// 不用深度缓冲
		
		vk::RenderingInfo ri = {};
		vk::Rect2D renderArea({ 0, 0 }, {sInOutExecuteInfo.nWidth, sInOutExecuteInfo.nHeight});
		ri
			.setRenderArea(renderArea)
			.setLayerCount(1)
			.setColorAttachments(vecColorAttachmentInfo)
			.setPDepthAttachment(nullptr)
			;

		cmdBuffer.BeginRendering(ri);


		std::array<vk::Viewport, 1> viewports;
		viewports[0]
			.setX(0)
			.setY(0)
			.setWidth(sInOutExecuteInfo.nWidth)
			.setHeight(sInOutExecuteInfo.nHeight)
			.setMinDepth(0.f)
			.setMaxDepth(1.f)
			;

		std::array<vk::Rect2D,1> scissors;
		scissors[0]
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(vk::Extent2D(sInOutExecuteInfo.nWidth, sInOutExecuteInfo.nHeight))
			;

		cmdBuffer.SetViewport(0, viewports);
		cmdBuffer.SetScissor(0, scissors);

		EntityDrawInfo sEntityDrawInfo;
		sEntityDrawInfo.pCommandBuffer = &cmdBuffer;

		for (auto* pEntity : sInOutExecuteInfo.mapRenderEntity[MATERIAL_ID_2D_0]) {
			EntityRenderLogLine* pEntityLogLine = dynamic_cast<EntityRenderLogLine*>(pEntity);
			if (pEntityLogLine)
			{
				pEntityLogLine->Draw(sEntityDrawInfo);
			}
		}

		cmdBuffer.EndRendering();
		cmdBuffer.End();

		CmdSubmitInfo sCmdSubmitInfo;

		// 开始阶段的Transiton的fence
		if (sStageBeginInfo.refFence) {
			// 等待开始阶段的sem
			sCmdSubmitInfo.vecSemToWait.push_back(m_arrSemTransitionBegin[nFlightFrameIndex]);
			sCmdSubmitInfo.vecSemWaitMasks.push_back(vk::PipelineStageFlagBits::eAllGraphics);
		}
		else
		{
			// 等待stage输入的sem
			sCmdSubmitInfo.vecSemToWait = sInOutExecuteInfo.vecSemWait;
			sCmdSubmitInfo.vecSemWaitMasks = sInOutExecuteInfo.vecSemWaitMask;
		}
		
		const bool bNeedOutTransition = OutNeedTransiton(sInOutExecuteInfo.vecRenderTarget);
		if (bNeedOutTransition)
		{
			// 如果需要执行输出的Transition，则发射结束阶段的sem
			sCmdSubmitInfo.vecSemToSignal.push_back(m_arrSemTransitionEnd[nFlightFrameIndex]);
		}
		else
		{
			// 如果不需要执行输出的Transition，则发射stage输出的sem
			sCmdSubmitInfo.vecSemToSignal = sInOutExecuteInfo.vecSemSignal;
		}

		FenceRef fenceDraw = cmdBuffer.Submit(sCmdSubmitInfo);

		// ------------------------- stage end -----------------------------
		if (bNeedOutTransition)
		{
			StageEndInfo sStageEndInfo;
			sStageEndInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
			sStageEndInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
			sStageEndInfo.nFlightFrameIndex = nFlightFrameIndex;
			sStageEndInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
			sStageEndInfo.vecSemWait.push_back(m_arrSemTransitionEnd[nFlightFrameIndex]);
			StageEnd(sStageEndInfo);

			sInOutExecuteInfo.refFence = sStageEndInfo.refFence;
		}
		else
		{
			sInOutExecuteInfo.refFence = fenceDraw;
		}
	}
} // namespace LT