#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "PipelineShade.hpp"

namespace LT {
	
	PipelineShade::PipelineShade() {
		vk::Device& device = vkContext::GetVkDevice();

		// 创建深度缓冲
		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++) {
			m_vecDepthBuffer.push_back(ImageManager::CreateImage2DDepthBuffer(m_nWidth, m_nHeight));
		}

		// 灯光
		// LightManager
		m_pLightManager = ShaderResourceManager::CreateLightManager();
		RENDERER_ASSERT(m_pLightManager, "Create Light Manager Failed.");

	}

	PipelineShade::~PipelineShade() {
		vk::Device& device = vkContext::GetVkDevice();
		//RENDERER_ASSERT(m_vecDepthBuffer.size() == RENDERER_DEFAULT_FLIGHT_FRAME_NUM, "func: %s. Depth Buffer Count Error.", __FUNCTION__);
		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++) {
			ImageManager::DeleteImage(m_vecDepthBuffer[i]);
		}
		m_pLightManager = nullptr;
	}

	void PipelineShade::Resize(uint32_t nWidth, uint32_t nHeight)
	{
		if (nWidth == 0 || nHeight == 0)
		{
			return;
		}
		if (m_nWidth == nWidth && m_nHeight == nHeight)
			return;

		m_nWidth = nWidth;
		m_nHeight = nHeight;

		if (m_vecDepthBuffer.size() > 0)
		{
			if (m_vecDepthBuffer.front()->GetWidth() == nWidth && m_vecDepthBuffer.front()->GetHeight() == nHeight)
			{
				return;
			}
		}

		for (auto* pDepthBuffer : m_vecDepthBuffer)
		{
			ImageManager::DeleteImage(pDepthBuffer);
		}
		for (size_t i = 0; i < m_vecDepthBuffer.size(); i++)
		{
			m_vecDepthBuffer[i] = ImageManager::CreateImage2DDepthBuffer(nWidth, nHeight);
		}
	}

	void PipelineShade::Execute(PipelineExecuteInfo& sInOutPipelineInfo)
	{
		// 更新Trans Buffer
		ShaderResourceManager::UpdateTransBuffer(
			sInOutPipelineInfo.pRenderView->GetTransBufferHandle(sInOutPipelineInfo.nFightFrameIndex), sInOutPipelineInfo.pRenderView->GetTransBuffer());
		ShaderResourceManager::UpdateDeviceTransBuffer();
		// 更新Light Uniform Buffer

		if (sInOutPipelineInfo.bLightDirty)
		{
			m_pLightManager->ClearLightBuffer();

			for (const auto& [sLightBuffer, bitsGroup] : sInOutPipelineInfo.vecLightBuffer) {
				m_pLightManager->PushLight(bitsGroup, sLightBuffer);
			}

			m_pLightManager->UpdateBufferToDevice();
		}

		// 更新Mtl Prop Buffer and Texture
		ShaderResourceManager::UpdateDeviceMtlPropBuffer();

		StageExecuteInfo sStageInfo;
		sStageInfo.vecRenderTarget = sInOutPipelineInfo.vecRenderTargets;
		sStageInfo.nDepthBufferID = m_vecDepthBuffer[sInOutPipelineInfo.nFightFrameIndex]->GetImageID();
		//sStageInfo.vecRenderEntity = sInOutPipelineInfo.vecEntityRender;
		sStageInfo.pRenderView = sInOutPipelineInfo.pRenderView;
		sStageInfo.nFlightFrameIndex = sInOutPipelineInfo.nFightFrameIndex;
		sStageInfo.nWidth = m_nWidth;
		sStageInfo.nHeight = m_nHeight;
		sStageInfo.pPipeline = this;
		sStageInfo.bDepthClear = true;
		sStageInfo.vecColorClear.push_back(true);

		if (!sInOutPipelineInfo.vecSemWait.empty())
		{
			sStageInfo.vecSemWait = sInOutPipelineInfo.vecSemWait;
			sStageInfo.vecSemWaitMask = sInOutPipelineInfo.vecSemWaitMask;
		}

		if (!sInOutPipelineInfo.vecSemSignal.empty())
		{
			sStageInfo.vecSemSignal = sInOutPipelineInfo.vecSemSignal;
		}

		// 遍历RenderEntity
		// 按照Material分类
		for (EntityRender* pEntity : sInOutPipelineInfo.vecEntityRender) {
			EntityRender3D* pEntity3D = reinterpret_cast<EntityRender3D*>(pEntity);
			sStageInfo.mapRenderEntity[pEntity3D->GetMaterialID()].push_back(pEntity);
		}

		m_cStage.Execute(sStageInfo);

		sInOutPipelineInfo.refFence = sStageInfo.refFence;
	}

	DescriptorSetWrapper* PipelineShade::GetLightBufferDescriptorSet()
	{
		if (m_pLightManager)
		{
			return &m_pLightManager->GetDescriptorSetWrapper();
		}
		else
		{
			return nullptr;
		}
	}

	ConstBufferHandle PipelineShade::GetLightBufferHandle()
	{
		if (m_pLightManager)
		{
			return m_pLightManager->GetBufferHandle();
		}
		else
		{
			return INVALID_CONST_BUFFER_HANDLE;
		}
	}

	void PipelineShade::SetColorInType(const std::vector<PipelineIOType>& vecColorInType)
	{
		Pipeline::SetColorInType(vecColorInType);
		m_cStage.SetColorInType(vecColorInType);
	}

	void PipelineShade::SetColorOutType(const std::vector<PipelineIOType>&vecColorOutType)
	{
		Pipeline::SetColorOutType(vecColorOutType);
		m_cStage.SetColorOutType(vecColorOutType);
	}

	void PipelineShade::SetDepthInType(PipelineIOType eDepthInType)
	{
		Pipeline::SetDepthInType(eDepthInType);
		m_cStage.SetDepthInType(eDepthInType);
	}

	void PipelineShade::SetDepthOutType(PipelineIOType eDepthOutType)
	{
		Pipeline::SetDepthOutType(eDepthOutType);
		m_cStage.SetDepthOutType(eDepthOutType);
	}

}
