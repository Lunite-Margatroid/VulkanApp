// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"
#include "MaterialManager.hpp"
#include "GraphicPass.hpp"

#include "RenderViewSingleCamera.hpp"

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
		if (sInOutExecuteInfo.mapRenderEntity.size() <= 0)
			return;
		FlightFrameIndex nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;

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

		//sStageBeginInfo.refFence.WaitFence();

		// -------------------------- Draw -------------------------------

		bool bFirstDraw = true;
		MaterialBindInfo sMtlBindInfo;
		sMtlBindInfo.nFlightIndex = nFlightFrameIndex;
		sMtlBindInfo.pCmdBuffer = &vkContext::GetCmdBuffer(nFlightFrameIndex);
		sMtlBindInfo.eStage = GetRenderStageType();
		// 遍历每个Material
		size_t nMtlCounter = 0;
		for (const auto& [nMtlID, vecEntityRender] : sInOutExecuteInfo.mapRenderEntity) {
			nMtlCounter += 1;
			bool bLastDraw = nMtlCounter == sInOutExecuteInfo.mapRenderEntity.size();
			StageDrawBeginInfo sBeginInfo;
			sBeginInfo.nHeight = sInOutExecuteInfo.nHeight;
			sBeginInfo.nWidth = sInOutExecuteInfo.nWidth;
			sBeginInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
			sBeginInfo.eColorLoadOp = bFirstDraw ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
			sBeginInfo.eDepthLoadOp = bFirstDraw ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
			sBeginInfo.eColorStoreOp = vk::AttachmentStoreOp::eStore;
			sBeginInfo.eDepthStoreOp = vk::AttachmentStoreOp::eStore;
			sBeginInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
			sBeginInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
			DrawBegin(sBeginInfo);

			IMaterial* pMaterial = MaterialManager::GetMaterial(nMtlID);
			if (RenderViewSingleCamera* pRenderView = dynamic_cast<RenderViewSingleCamera*>(sInOutExecuteInfo.pRenderView))
			{
				pMaterial->SetTransBuffer(pRenderView->GetTransBufferHandle(nFlightFrameIndex));
			}

			//sDrawInfo.nWidth = sInOutExecuteInfo.nWidth;
			//sDrawInfo.nHeight = sInOutExecuteInfo.nHeight;
			//sDrawInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
			//sDrawInfo.nDepthBuffer = sInOutExecuteInfo.nDepthBufferID;
			//sDrawInfo.vecRenderTargets = sInOutExecuteInfo.vecRenderTarget;
			//sDrawInfo.pRenderView = sInOutExecuteInfo.pRenderView;
			//sDrawInfo.eRenderStage = GetRenderStageType();
			//sDrawInfo.bClear = bFirstDraw;
			// 这里会更新DescriptorSet
			// UpdateDescriptorSets会被调用
			pMaterial->BindShaderResource(sMtlBindInfo);

			// 把Entity按照RenderPassFlag排序
			std::vector<std::pair<RenderPassFlag, EntityRender*>> entities;
			for (EntityRender* pEntity : vecEntityRender)
			{
				entities.emplace_back(
					pEntity->GetRenderPassFlag(), pEntity);
			}

			std::sort(entities.begin(), entities.end(),
				[](const std::pair<RenderPassFlag, EntityRender*>& a, const std::pair<RenderPassFlag, EntityRender*>& b) {
					return static_cast<int64_t>(a.first) < static_cast<int64_t>(b.first);
				});
			// 遍历Entity
			// 当RenderPassFlag改变时重新绑定Mateiral
			sMtlBindInfo.nFlag = entities.front().first;
			RenderPassFlag nLastFlag = sMtlBindInfo.nFlag;
			// DescriptorSet在这里绑定
			GraphicPass* pGraphicPass = dynamic_cast<GraphicPass*>(pMaterial->Bind(sMtlBindInfo));
			for (const auto &[nFlag, pEntity] : entities) {
				if (nLastFlag != nFlag)
				{
					sMtlBindInfo.nFlag = entities.front().first;
					pMaterial->Bind(sMtlBindInfo);
					nLastFlag = nFlag;
				}
				EntityDrawInfo sDrawInfo;
				sDrawInfo.pCommandBuffer = &vkContext::GetCmdBuffer(sInOutExecuteInfo.nFlightFrameIndex);
				if (pGraphicPass)
				{
					sDrawInfo.vkLayout = pGraphicPass->GetPipelineLayout();
				}

				pEntity->Draw(sDrawInfo);
			}

			StageDrawEndInfo sEndInfo;
			sEndInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
			if (bFirstDraw)
			{
				// 等待Transition
				sEndInfo.vecSemWait.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
				sEndInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllCommands);
			}

			// 发射Semaphore 让Transition等待
			if (bLastDraw) {
				sEndInfo.vecSemSignal.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);
			}

			DrawEnd(sEndInfo);

			bFirstDraw = false;
		}

		//EntityDrawInfo sDrawInfo;
		//sDrawInfo.nWidth = sInOutExecuteInfo.nWidth;
		//sDrawInfo.nHeight = sInOutExecuteInfo.nHeight;
		//sDrawInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		//sDrawInfo.nDepthBuffer = sInOutExecuteInfo.nDepthBufferID;
		//sDrawInfo.vecRenderTargets = sInOutExecuteInfo.vecRenderTarget;
		//sDrawInfo.pRenderView = sInOutExecuteInfo.pRenderView;
		//sDrawInfo.eRenderStage = GetRenderStageType();
		//sDrawInfo.bClear = true;

		//if (sInOutExecuteInfo.vecRenderEntity.size() >= 2)
		//{
		//	// 第一个渲染实体
		//	// 等待Transition
		//	sDrawInfo.vecSemWait.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
		//	sDrawInfo.vecSemWaitMasks.push_back(vk::PipelineStageFlagBits::eAllCommands);

		//	sInOutExecuteInfo.vecRenderEntity.front()->Draw(sDrawInfo);

		//	sDrawInfo.vecSemWait.clear();
		//	sDrawInfo.vecSemWaitMasks.clear();

		//	sDrawInfo.bClear = false;
		//	// 第2到n个渲染实体
		//	for (int i = 1; i < sInOutExecuteInfo.vecRenderEntity.size() - 1; i++)
		//	{
		//		sInOutExecuteInfo.vecRenderEntity[i]->Draw(sDrawInfo);
		//	}
		//	// 最后一个渲染实体

		//	sDrawInfo.vecSemSignal.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);

		//	sInOutExecuteInfo.vecRenderEntity.back()->Draw(sDrawInfo);
		//}
		//else if(sInOutExecuteInfo.vecRenderEntity.size() > 0)
		//{
		//	// 等待Transition
		//	sDrawInfo.vecSemWait.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
		//	sDrawInfo.vecSemWaitMasks.push_back(vk::PipelineStageFlagBits::eAllCommands);

		//	sDrawInfo.vecSemSignal.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);

		//	sInOutExecuteInfo.vecRenderEntity.front()->Draw(sDrawInfo);
		//}
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
	void RenderStageOpaqueForward::DrawBegin(const StageDrawBeginInfo& sBeginInfo)
	{
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(sBeginInfo.nFlightFrameIndex);
		vk::Extent2D extent = { sBeginInfo.nWidth , sBeginInfo.nHeight };
		uint32_t nFlightFrameIndex = sBeginInfo.nFlightFrameIndex;


		vk::CommandBufferBeginInfo cbbi = {};
		cmdBuffer.Begin(cbbi);

		// 颜色
		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::RenderingAttachmentInfo rai;
		rai
			.setImageView(ImageManager::GetInstance().GetNativeDeviceImageView(sBeginInfo.vecRenderTarget[0]))
			.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(sBeginInfo.eColorLoadOp)
			.setStoreOp(sBeginInfo.eColorStoreOp)
			.setClearValue(clearColor)
			;

		// 深度
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
		vk::RenderingAttachmentInfo raiDepth;
		raiDepth
			.setImageView(ImageManager::GetInstance().GetNativeDeviceImageView(sBeginInfo.nDepthBufferID))
			.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
			.setLoadOp(sBeginInfo.eDepthLoadOp)
			.setStoreOp(sBeginInfo.eDepthStoreOp)
			.setClearValue(clearDepth)
			;

		// 录入渲染命令
		vk::RenderingInfo ri;

		vk::Rect2D renderArea({ 0, 0 }, { sBeginInfo.nWidth, sBeginInfo.nHeight });
		ri
			.setRenderArea(renderArea)
			.setLayerCount(1)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&rai)
			.setPDepthAttachment(&raiDepth)
			;

		cmdBuffer.BeginRendering(ri);


		// 创建并绑定Viewport Scissor State
		vk::Viewport viewport;
		viewport
			.setX(0)
			.setY(0)
			.setWidth(sBeginInfo.nWidth)
			.setHeight(sBeginInfo.nHeight)
			.setMinDepth(0.f)
			.setMaxDepth(1.f)
			;

		// Scissor
		vk::Rect2D scissor;
		scissor
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(vk::Extent2D(sBeginInfo.nWidth, sBeginInfo.nHeight))
			;

		cmdBuffer.SetViewport(0, std::vector<vk::Viewport>{viewport});
		cmdBuffer.SetScissor(0, std::vector<vk::Rect2D>{scissor});
	}
	void RenderStageOpaqueForward::DrawEnd(StageDrawEndInfo& sEndInfo)
	{
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(sEndInfo.nFlightFrameIndex);

		cmdBuffer.EndRendering();
		cmdBuffer.End();


		CmdSubmitInfo sCmdSubmitInfo;


		// 需要等待的信号量
		if (sEndInfo.vecSemWait.size() > 0)
		{
			sCmdSubmitInfo.vecSemToWait = sEndInfo.vecSemWait;
			sCmdSubmitInfo.vecSemWaitMasks = sEndInfo.vecSemWaitMask;
		}
		// 需要发射的信号量
		if (sEndInfo.vecSemSignal.size() > 0)
		{
			sCmdSubmitInfo.vecSemToSignal = sEndInfo.vecSemSignal;
		}


		FenceRef fence = cmdBuffer.Submit(sCmdSubmitInfo);

		sEndInfo.refFence = fence;
	}
} // namespace LT