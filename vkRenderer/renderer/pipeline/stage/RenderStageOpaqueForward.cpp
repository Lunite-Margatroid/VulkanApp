// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"
#include "MaterialManager.hpp"
#include "GraphicPass.hpp"

#include "RenderViewSingleCamera.hpp"

#include "PipelineShade.hpp"

#include "SemaphoreManager.hpp"

namespace LT {
	RenderStageOpaqueForward::RenderStageOpaqueForward()
	{
	}
	RenderStageOpaqueForward::~RenderStageOpaqueForward()
	{
	}
	void RenderStageOpaqueForward::Execute(StageExecuteInfo& sInOutExecuteInfo)
	{
		if (sInOutExecuteInfo.mapRenderEntity.size() <= 0)
			return;
		const FlightFrameIndex nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		const bool bNeedOutTransition = OutNeedTransiton(sInOutExecuteInfo.vecRenderTarget);

		// ----------------------------- Stage Begin------------------------
		// Barrier
		// 进行Transition
		StageBeginInfo sStageBeginInfo;
		sStageBeginInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
		sStageBeginInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
		sStageBeginInfo.nFlightFrameIndex = nFlightFrameIndex;
		//sStageBeginInfo.vecSemWait = sInOutExecuteInfo.vecSemWait;
		//sStageBeginInfo.vecSemWait = barrierStageBegin.GetWaitSemaphores();
		//sStageBeginInfo.vecSemWaitMask = sInOutExecuteInfo.vecSemWaitMask;
		//sStageBeginInfo.vecSemSignal.push_back(m_arrSemTransitionBegin[nFlightFrameIndex]);
		//sStageBeginInfo.vecSemSignal.push_back(barrierStageBegin.GetSignalSemaphore());
		sStageBeginInfo.refFence = sInOutExecuteInfo.refFence;
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
		FenceRef fenceDraw = sStageBeginInfo.refFence;
		for (const auto& [nMtlID, vecEntityRender] : sInOutExecuteInfo.mapRenderEntity) {
			// 跳过无效材质ID和特殊材质
			if (nMtlID <= INVALID_MATERIAL_ID)
				continue;
			nMtlCounter += 1;
			bool bLastDraw = nMtlCounter == sInOutExecuteInfo.mapRenderEntity.size();
			StageDrawBeginInfo sBeginInfo;
			sBeginInfo.nHeight = sInOutExecuteInfo.nHeight;
			sBeginInfo.nWidth = sInOutExecuteInfo.nWidth;
			sBeginInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
			sBeginInfo.eDepthLoadOp = (bFirstDraw && sInOutExecuteInfo.bDepthClear) ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
			sBeginInfo.eDepthStoreOp = vk::AttachmentStoreOp::eStore;
			sBeginInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
			sBeginInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;

			for (int i = 0; i < sInOutExecuteInfo.vecRenderTarget.size(); ++i)
			{
				sBeginInfo.vecColorLoadOp.push_back((bFirstDraw && sInOutExecuteInfo.vecColorClear[i]) ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad);
				sBeginInfo.vecColorStoreOp.push_back(vk::AttachmentStoreOp::eStore);
			}

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
			RenderPassFlag nLastFlag = static_cast<uint64_t>(RenderPassFlagBits::eInvalid);
			GraphicPass* pGraphicPass = nullptr;
			for (const auto &[nFlag, pEntity] : entities) {
				if (nLastFlag != nFlag)
				{
					sMtlBindInfo.nFlag = entities.front().first;
					// 这里会更新DescriptorSet
					// UpdateDescriptorSets会被调用
					pMaterial->BindShaderResource(sMtlBindInfo);

					// DescriptorSet在这里绑定
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
			//if (bFirstDraw)
			//{
			//	// 等待Transition
			//	sEndInfo.vecSemWait.push_back(m_arrSemTransitionBegin[sInOutExecuteInfo.nFlightFrameIndex]);
			//	sEndInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllCommands);
			//}

			//if (bLastDraw) {
			//	if (bNeedOutTransition)
			//	{
			//		// 发射Semaphore 让Transition等待
			//		sEndInfo.vecSemSignal.push_back(m_arrSemTransitionEnd[sInOutExecuteInfo.nFlightFrameIndex]);
			//	}
			//	else
			//	{
			//		// 如果不需要Transition 那么就把外部的Semaphore发射出去
			//		sEndInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
			//	}
			//}

			auto barrierDraw = SemaphoreManager::Barrier(nFlightFrameIndex, fenceDraw);

			sEndInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllCommands);
			sEndInfo.vecSemWait = barrierDraw.GetWaitSemaphores();
			sEndInfo.vecSemSignal.push_back(barrierDraw.GetSignalSemaphore());

			DrawEnd(sEndInfo);

			fenceDraw = sEndInfo.refFence;
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
		if (bNeedOutTransition)
		{
			SemaphoreBarrier barrierStageEnd = SemaphoreManager::Barrier(nFlightFrameIndex, fenceDraw);

			StageEndInfo sStageEndInfo;
			sStageEndInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
			sStageEndInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
			sStageEndInfo.nFlightFrameIndex = nFlightFrameIndex;
			//sStageEndInfo.vecSemWait.push_back(m_arrSemTransitionEnd[nFlightFrameIndex]);
			sStageEndInfo.vecSemWait = barrierStageEnd.GetWaitSemaphores();
			sStageEndInfo.vecSemWaitMask.resize(sStageEndInfo.vecSemWait.size(), vk::PipelineStageFlagBits::eAllGraphics);
			//if (!sInOutExecuteInfo.vecSemSignal.empty())
			//{
			//	sStageEndInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
			//}
			StageEnd(sStageEndInfo);

			sInOutExecuteInfo.refFence = sStageEndInfo.refFence;
		}
		else
		{
			sInOutExecuteInfo.refFence = fenceDraw;
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

		std::vector<vk::RenderingAttachmentInfo> vecColorAttackmentInfo;
		for (int i = 0; i < sBeginInfo.vecRenderTarget.size(); i++)
		{
			vk::RenderingAttachmentInfo rai;
			rai
				.setImageView(ImageManager::GetInstance().GetNativeDeviceImageView(sBeginInfo.vecRenderTarget[i]))
				.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
				.setLoadOp(sBeginInfo.vecColorLoadOp[i])
				.setStoreOp(sBeginInfo.vecColorStoreOp[i])
				.setClearValue(clearColor)
				;
			vecColorAttackmentInfo.push_back(rai);
		}
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
			.setColorAttachments(vecColorAttackmentInfo)
			.setPDepthAttachment(&raiDepth)
			;

		cmdBuffer.BeginRendering(ri);


		// 创建并绑定Viewport Scissor State
		std::array<vk::Viewport, 1> viewports;
		viewports[0]
			.setX(0)
			.setY(0)
			.setWidth(sBeginInfo.nWidth)
			.setHeight(sBeginInfo.nHeight)
			.setMinDepth(0.f)
			.setMaxDepth(1.f)
			;

		// Scissor
		std::array<vk::Rect2D,1> scissors;
		scissors[0]
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(vk::Extent2D(sBeginInfo.nWidth, sBeginInfo.nHeight))
			;

		cmdBuffer.SetViewport(0, viewports);
		cmdBuffer.SetScissor(0, scissors);
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