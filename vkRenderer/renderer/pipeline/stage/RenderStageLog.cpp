#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "RenderStageLog.hpp"
#include "BufferManager.h"
#include "ImageManager.h"

#include "EntityRenderLogLine.hpp"

#include "PipelineLog.hpp"

namespace LT {
	RenderStageLog::RenderStageLog()
		: m_nASCIITex(INVALID_IMAGE_ID)

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
			sizeof(ModelMat2D)
		);

		{
			m_pass.AddShaderModule("CommonVertexShader2D", {});
			m_pass.AddShaderModule("FragmentShaderLogLine", {});
		}

		m_pass.Init({ m_vk2DTransBufferDescSetLayout }, vecPushConstRange);


		m_sVPMat2D.offset = glm::vec2(0.f, 0.f);
		m_sVPMat2D.vpMat = glm::mat2(1.f,0.f,1.f,0.f);
		for (int i = 0; i < m_arrTransBuffer2D.size(); ++i)
		{
			m_arrTransBuffer2D[i] = BufferManager::CreateConstBuffer(sizeof(VPMatrixBuffer2D), &m_sVPMat2D);
		}
	}
	RenderStageLog::~RenderStageLog()
	{
		VK_CONTEXT_DEVICE;

		for (int i = 0; i < m_arrTransBuffer2D.size(); ++i)
		{
			BufferManager::DeleteBuffer(m_arrTransBuffer2D[i]);
		}
		device.destroyDescriptorSetLayout(m_vk2DTransBufferDescSetLayout);
	}
	void LT::RenderStageLog::Execute(StageExecuteInfo& sInOutExecuteInfo)
	{
		if (sInOutExecuteInfo.mapRenderEntity.size() <= 0) return;

		const FlightFrameIndex nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(nFlightFrameIndex);

		// UpdateDescriptorSet
		{
			std::array<vk::WriteDescriptorSet, 2> arrWDS = {};
			vk::DescriptorBufferInfo dbi = {};
			dbi
				.setBuffer(m_arrTransBuffer2D[nFlightFrameIndex]->GetNativeBuffer())
				.setOffset(0)
				.setRange(m_arrTransBuffer2D[nFlightFrameIndex]->Size())
				;
			arrWDS[0]
				.setPBufferInfo(&dbi)
				.setDstBinding(TRANS_2D_BUFFER_BINDING_INDEX)
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				;

			vk::DescriptorImageInfo dii = {};

			dii
				.setImageView(ImageManager::GetNativeDeviceImageView(m_nASCIITex))
				.setSampler(SamplerManager::GetDefaultImageSampler()->GetNativeSampler())
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				;
			arrWDS[1]
				.setPImageInfo(&dii)
				.setDstBinding(LOG_PASS_TEXTURE_BINDING_INDEX)
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				;

			m_arrDescSet[nFlightFrameIndex].UpdateDescriptorSets(arrWDS);
		}

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

		// Bind Descriptor Set
		m_pass.SetDescriptorSets({&m_arrDescSet[nFlightFrameIndex]});
		m_pass.Bind(&cmdBuffer);

		EntityDrawInfo sEntityDrawInfo;
		sEntityDrawInfo.pCommandBuffer = &cmdBuffer;
		sEntityDrawInfo.vkLayout = m_pass.GetPipelineLayout();

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


	RenderStageType RenderStageLog::GetRenderStageType() const {
		return RenderStageType::eLog;
	}
	void RenderStageLog::SetASCIITex(ImageID nASCIITex)
	{
		m_nASCIITex = nASCIITex;
	}
} // namespace LT