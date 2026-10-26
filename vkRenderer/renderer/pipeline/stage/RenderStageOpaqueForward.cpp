// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"
#include "MaterialManager.hpp"
#include "GraphicPass.hpp"

#include "RenderViewSingleCamera.hpp"

#include "PipelineShade.hpp"

namespace LT {
	RenderStageOpaqueForward::RenderStageOpaqueForward()
	{

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
		sMtlBindInfo.pPipeline = dynamic_cast<PipelineShade*>(sInOutExecuteInfo.pPipeline);
		RENDERER_ASSERT(sMtlBindInfo.pPipeline, "Bad Pipeline Type");
		// Draw End Info
		StageDrawEndInfo sEndInfo;

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

			// 该材质受光照影响
			if (HasLighting(pMaterial->GetMaterialFlag()))
			{
				pMaterial->SetLightBuffer(sMtlBindInfo.pPipeline->GetLightBufferHandle());
			}

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
					pGraphicPass = dynamic_cast<GraphicPass*>(pMaterial->Bind(sMtlBindInfo));
					nLastFlag = nFlag;
				}
				EntityDrawInfo sDrawInfo;
				sDrawInfo.pCommandBuffer = &vkContext::GetCmdBuffer(sInOutExecuteInfo.nFlightFrameIndex);
				if (!pGraphicPass)
				{
					pGraphicPass = dynamic_cast<GraphicPass*>(MaterialManager::GetInstance().GetDefaultErrorMaterial().GetPtr()->Bind(sMtlBindInfo));
					RENDERER_ASSERT(pGraphicPass, "Failed to bind error material");

				}

				sDrawInfo.vkLayout = pGraphicPass->GetPipelineLayout();

				pEntity->Draw(sDrawInfo);
			}

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

		// 如果End的Fence是空的 把Draw的Fence返回
		if (sStageEndInfo.refFence)
		{
			sInOutExecuteInfo.refFence = sStageEndInfo.refFence;
		}
		else
		{
			sInOutExecuteInfo.refFence = sEndInfo.refFence;
		}
	}
	void RenderStageOpaqueForward::StageBegin(StageBeginInfo& sInOutBeginInfo)
	{

		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(sInOutBeginInfo.nFlightFrameIndex);

		RecordTransitionInfo sRecordInfo;
		sRecordInfo.pCmdBuffer = &cmdBuffer;

		// Color Attacment
		uint32_t nColorAttachmentCount = std::min(sInOutBeginInfo.vecRenderTarget.size(), m_vecColorInType.size());
		for (int i = 0; i < nColorAttachmentCount; ++i)
		{
			PipelineIOType eInColorType = m_vecColorInType[i];

			if (eInColorType != PipelineIOType::eColorAttachment)
			{
				ImageID nImageID = sInOutBeginInfo.vecRenderTarget[i];

				sRecordInfo.vecColorAttachment.emplace_back(eInColorType, PipelineIOType::eColorAttachment, nImageID);
			}
		}


		// Depth Attachment
		if (sInOutBeginInfo.nDepthBufferID != INVALID_IMAGE_ID)
		{
			if (m_eDepthInType != PipelineIOType::eDepthAttachment)
			{
				sRecordInfo.vecDepthAttachment.emplace_back(m_eDepthInType, PipelineIOType::eDepthAttachment, sInOutBeginInfo.nDepthBufferID);
			}
		}

		// 如果有Image需要Transition
		if (sRecordInfo.vecColorAttachment.size() + sRecordInfo.vecDepthAttachment.size() > 0)
		{

			m_passTransition.RecordCommand(sRecordInfo);

			TransitionSubmitInfo sSubmitInfo;
			sSubmitInfo.nFlightFrameIndex = sInOutBeginInfo.nFlightFrameIndex;
			sSubmitInfo.vecSemToSignal = sInOutBeginInfo.vecSemSignal;
			sSubmitInfo.vecSemToWait = sInOutBeginInfo.vecSemWait;
			sSubmitInfo.vecSemWaitMasks = sInOutBeginInfo.vecSemWaitMask;

			m_passTransition.Submit(sSubmitInfo);

			sInOutBeginInfo.refFence = sSubmitInfo.refFence;
		}
	}

	void RenderStageOpaqueForward::StageEnd(StageEndInfo& sInOutEndInfo) {
		RecordTransitionInfo sRecordInfo;
		sRecordInfo.pCmdBuffer = &vkContext::GetCmdBuffer(sInOutEndInfo.nFlightFrameIndex);
		// Color Attachment
		uint32_t nColorAttachmentCount = std::min(sInOutEndInfo.vecRenderTarget.size(), m_vecColorInType.size());
		for (int i = 0; i < nColorAttachmentCount; ++i) {
			PipelineIOType eOutColorType = m_vecColorOutType[i];

			if (eOutColorType != PipelineIOType::eColorAttachment && eOutColorType != PipelineIOType::eUndefined)
			{
				ImageID nImageID = sInOutEndInfo.vecRenderTarget[i];

				sRecordInfo.vecColorAttachment.emplace_back(PipelineIOType::eColorAttachment, eOutColorType, nImageID);
			}
		}

		// Depth Attachment
		if (m_eDepthOutType != PipelineIOType::eUndefined && m_eDepthOutType != PipelineIOType::eDepthAttachment) {
			sRecordInfo.vecColorAttachment.emplace_back(PipelineIOType::eDepthAttachment, m_eDepthOutType, sInOutEndInfo.nDepthBufferID);
		}

		if (sRecordInfo.vecColorAttachment.size() + sRecordInfo.vecDepthAttachment.size() > 0)
		{

			m_passTransition.RecordCommand(sRecordInfo);

			TransitionSubmitInfo sSubmitInfo;
			sSubmitInfo.nFlightFrameIndex = sInOutEndInfo.nFlightFrameIndex;
			sSubmitInfo.vecSemToSignal = sInOutEndInfo.vecSemSignal;
			sSubmitInfo.vecSemToWait = sInOutEndInfo.vecSemWait;
			sSubmitInfo.vecSemWaitMasks = sInOutEndInfo.vecSemWaitMask;

			m_passTransition.Submit(sSubmitInfo);

			sInOutEndInfo.refFence = sSubmitInfo.refFence;
		}
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