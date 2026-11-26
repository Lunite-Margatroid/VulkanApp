#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "RenderStageLog.hpp"
#include "BufferManager.h"
#include "ImageManager.h"

#include "EntityRenderLogLine.hpp"

#include "PipelineLog.hpp"

#include "SemaphoreManager.hpp"

namespace LT {
	RenderStageLog::RenderStageLog()
		: m_nASCIITex(INVALID_IMAGE_ID)
		, m_bVPMat2DDirt(true)
	{
		VK_CONTEXT_DEVICE;

		std::array<vk::DescriptorSetLayoutBinding, 2> bindings;
		// Trans Buffer 2D
		bindings[0]
			.setBinding(TRANS_2D_BUFFER_BINDING_INDEX)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(GetShaderStageFlag(TRANS_2D_BUFFER_BINDING_SPACE))
			;
		// Text Texture
		bindings[1]
			.setBinding(LOG_PASS_TEXTURE_BINDING_INDEX)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setStageFlags(GetShaderStageFlag(LOG_PASS_TEXTURE_BINDING_SPACE))
			;

		vk::DescriptorSetLayoutCreateInfo dslci = {};
		dslci.setBindings(bindings);

		m_vk2DTransBufferDescSetLayout = device.createDescriptorSetLayout(dslci);


		for (int i = 0; i < m_arrDescSet.size(); ++i) {
			m_arrDescSet[i].Init(ShaderResourceManager::GetDescriptorPool(), m_vk2DTransBufferDescSetLayout);
		}

		std::vector<vk::PushConstantRange> vecPushConstRange;
		vecPushConstRange.emplace_back(
			vk::ShaderStageFlagBits::eVertex,
			0,
			static_cast<uint32_t>(sizeof(ModelMat2D))
		);

		{
			m_pass.AddShaderModule("CommonVertexShader2D", {});
			m_pass.AddShaderModule("FragmentShaderLogLine", {});
		}

		m_pass.Init({ m_vk2DTransBufferDescSetLayout }, vecPushConstRange);


		m_sVPMat2D.offset = glm::vec2(0.f, 0.f);
		m_sVPMat2D.vpMat = glm::mat2(1.f / 1024.f,0.f,0.f,1.f / 1024.f);
		for (int i = 0; i < m_arrTransBuffer2D.size(); ++i)
		{
			m_arrTransBuffer2D[i] = ShaderResourceManager::CreateCommonConstBuffer(sizeof(VPMatrixBuffer2D), &m_sVPMat2D);
		}
	}
	RenderStageLog::~RenderStageLog()
	{
		VK_CONTEXT_DEVICE;

		for (int i = 0; i < m_arrDescSet.size(); ++i) {
			m_arrDescSet[i].Destroy(ShaderResourceManager::GetDescriptorPool());
		}

		for (int i = 0; i < m_arrTransBuffer2D.size(); ++i)
		{
			ShaderResourceManager::ReleaseCommonConstBuffer(m_arrTransBuffer2D[i]);
		}
		device.destroyDescriptorSetLayout(m_vk2DTransBufferDescSetLayout);
	}
	void LT::RenderStageLog::Execute(StageExecuteInfo& sInOutExecuteInfo)
	{
		if (sInOutExecuteInfo.mapRenderEntity.size() <= 0) return;

		const FlightFrameIndex nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(nFlightFrameIndex);

		if (m_bVPMat2DDirt)
		{
			m_sVPMat2D.vpMat = m_camera.GetVPMat();
			m_sVPMat2D.offset = m_camera.GetOffset();
			m_bVPMat2DDirt = false;
		}

		ShaderResourceManager::UpdateCommonConstBuffer(m_arrTransBuffer2D[nFlightFrameIndex], &m_sVPMat2D);

		// UpdateDescriptorSet
		m_pass.SetDescriptorSets({ &m_arrDescSet[nFlightFrameIndex] });
		m_pass.BindConstBuffer(m_arrTransBuffer2D[nFlightFrameIndex], TRANS_2D_BUFFER_BINDING_INDEX, 0, ShaderStage::eVertexShader);
		m_pass.BindImage(m_nASCIITex, LOG_PASS_TEXTURE_BINDING_INDEX, 0, ShaderStage::eFragmentShader);
		m_pass.UpdateDescriptorSet();
		// ----------------------------- Stage Begin------------------------
		// Barrier
		// 进行Transition

		StageBeginInfo sStageBeginInfo;
		sStageBeginInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
		sStageBeginInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
		sStageBeginInfo.nFlightFrameIndex = nFlightFrameIndex;
		//sStageBeginInfo.vecSemWait = sInOutExecuteInfo.vecSemWait;
		//sStageBeginInfo.vecSemWaitMask = sInOutExecuteInfo.vecSemWaitMask;
		//sStageBeginInfo.vecSemWait = barrierStageBegin.GetWaitSemaphores();
		//sStageBeginInfo.vecSemWaitMask.resize(sStageBeginInfo.vecSemWait.size(), vk::PipelineStageFlagBits::eAllGraphics);
		//sStageBeginInfo.vecSemSignal.push_back(barrierStageBegin.GetSignalSemaphore());
		sStageBeginInfo.refFence = sInOutExecuteInfo.refFence;
		//sStageBeginInfo.vecSemSignal.push_back(m_arrSemTransitionBegin[nFlightFrameIndex]);
		StageBegin(sStageBeginInfo);
		sInOutExecuteInfo.refFence = sStageBeginInfo.refFence;
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

		// Bind Descriptor Set
		m_pass.Bind(&cmdBuffer);

		EntityDrawInfo sEntityDrawInfo;
		sEntityDrawInfo.pCommandBuffer = &cmdBuffer;
		sEntityDrawInfo.vkLayout = m_pass.GetPipelineLayout();

		glm::mat3 modelMat(1.f);
		glm::vec3 offset(0.f);


		const auto& entities = sInOutExecuteInfo.mapRenderEntity[MATERIAL_ID_2D_0];
		for (auto iter = entities.rbegin(); iter != entities.rend(); ++iter)
		{
			EntityRenderLogLine* pEntityLogLine = dynamic_cast<EntityRenderLogLine*>(*iter);
			if (pEntityLogLine)
			{
				modelMat[2] = offset;
				pEntityLogLine->SetModuleMat(glm::value_ptr(modelMat));
				pEntityLogLine->Draw(sEntityDrawInfo);
				offset.y += LOG_PASS_DEFAULT_LINE_HEIGHT;
			}
		}
		cmdBuffer.EndRendering();
		cmdBuffer.End();

		CmdSubmitInfo sCmdSubmitInfo;

		// 开始阶段的Transiton的fence
		//if (sStageBeginInfo.refFence) {
		//	// 等待开始阶段的sem
		//	sCmdSubmitInfo.vecSemToWait.push_back(m_arrSemTransitionBegin[nFlightFrameIndex]);
		//	sCmdSubmitInfo.vecSemWaitMasks.push_back(vk::PipelineStageFlagBits::eAllGraphics);
		//}
		//else
		//{
		//	// 等待stage输入的sem
		//	sCmdSubmitInfo.vecSemToWait = sInOutExecuteInfo.vecSemWait;
		//	sCmdSubmitInfo.vecSemWaitMasks = sInOutExecuteInfo.vecSemWaitMask;
		//}
		//
		const bool bNeedOutTransition = OutNeedTransiton(sInOutExecuteInfo.vecRenderTarget);
		//if (bNeedOutTransition)
		//{
		//	// 如果需要执行输出的Transition，则发射结束阶段的sem
		//	sCmdSubmitInfo.vecSemToSignal.push_back(m_arrSemTransitionEnd[nFlightFrameIndex]);
		//}
		//else
		//{
		//	// 如果不需要执行输出的Transition，则发射stage输出的sem
		//	sCmdSubmitInfo.vecSemToSignal = sInOutExecuteInfo.vecSemSignal;
		//}

		SemaphoreBarrier barrierDraw = SemaphoreManager::Barrier(nFlightFrameIndex, sStageBeginInfo.refFence);
		sCmdSubmitInfo.vecSemToSignal.push_back(barrierDraw.GetSignalSemaphore());
		sCmdSubmitInfo.vecSemToWait = barrierDraw.GetWaitSemaphores();
		sCmdSubmitInfo.vecSemWaitMasks.resize(sCmdSubmitInfo.vecSemToWait.size(), vk::PipelineStageFlagBits::eAllGraphics);

		FenceRef fenceDraw = cmdBuffer.Submit(sCmdSubmitInfo);

		// ------------------------- stage end -----------------------------
		if (bNeedOutTransition)
		{
			SemaphoreBarrier barrierStageEnd = SemaphoreManager::Barrier(nFlightFrameIndex, fenceDraw);

			StageEndInfo sStageEndInfo;
			sStageEndInfo.nDepthBufferID = sInOutExecuteInfo.nDepthBufferID;
			sStageEndInfo.vecRenderTarget = sInOutExecuteInfo.vecRenderTarget;
			sStageEndInfo.nFlightFrameIndex = nFlightFrameIndex;
			//sStageEndInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
			//sStageEndInfo.vecSemWait.push_back(m_arrSemTransitionEnd[nFlightFrameIndex]);
			//sStageEndInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllGraphics);
			sStageEndInfo.vecSemSignal.push_back(barrierStageEnd.GetSignalSemaphore());
			sStageEndInfo.vecSemWait = barrierStageEnd.GetWaitSemaphores();
			sStageEndInfo.vecSemWaitMask.resize(sStageEndInfo.vecSemWait.size(), vk::PipelineStageFlagBits::eAllGraphics);
			StageEnd(sStageEndInfo);

			sInOutExecuteInfo.refFence = sStageEndInfo.refFence;
		}
		else
		{
			sInOutExecuteInfo.refFence = fenceDraw;
		}
	}


	RenderStageType RenderStageLog::GetRenderStageType() const {
		return RenderStageType::eLog;
	}
	void RenderStageLog::SetASCIITex(ImageID nASCIITex)
	{
		m_nASCIITex = nASCIITex;
	}
	void RenderStageLog::UpdateCamera(uint32_t nWidth, uint32_t nHeight)
	{
		float fWidth = static_cast<float>(nWidth);
		float fHeight = static_cast<float>(nHeight);

		float fAspect = fWidth / fHeight;

		m_camera.SetWidth(static_cast<float>(fWidth));
		m_camera.SetAspect(fAspect);

		m_camera.SetPosition(glm::vec2(fWidth / 2, fHeight / 2));

		m_bVPMat2DDirt = true;
	}
} // namespace LT