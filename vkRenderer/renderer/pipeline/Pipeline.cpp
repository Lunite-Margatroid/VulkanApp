#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "SlangCompiler.hpp"
#include "IBindable.h"

#include "ImageSampler.h"


namespace LT {
	Pipeline::Pipeline()
		:m_nWidth(DEFAULT_WIDTH)
		,m_nHeight(DEFAULT_HEIGHT)
	{

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

	Pipeline::~Pipeline() {
		vk::Device& device = vkContext::GetVkDevice();
		//RENDERER_ASSERT(m_vecDepthBuffer.size() == RENDERER_DEFAULT_FLIGHT_FRAME_NUM, "func: %s. Depth Buffer Count Error.", __FUNCTION__);
		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++) {
			ImageManager::DeleteImage(m_vecDepthBuffer[i]);
		}


	}

	void Pipeline::Resize(uint32_t nWidth, uint32_t nHeight)
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

	void Pipeline::Execute(FrameInfo& sInOutFrameInfo)
	{
		// 更新Trans Buffer
		ShaderResourceManager::UpdateTransBuffer(
			sInOutFrameInfo.pRenderView->GetTransBufferHandle(sInOutFrameInfo.nFightFrameIndex), sInOutFrameInfo.pRenderView->GetTransBuffer());
		ShaderResourceManager::UpdateDeviceTransBuffer();
		// 更新Light Uniform Buffer

		if(sInOutFrameInfo.bLightDirty)
		{
			m_pLightManager->ClearLightBuffer();

			for (const auto& [sLightBuffer, bitsGroup] : sInOutFrameInfo.vecLightBuffer) {
				m_pLightManager->PushLight(bitsGroup, sLightBuffer);
			}

			m_pLightManager->UpdateBufferToDevice();
		}

		// 更新Mtl Prop Buffer and Texture
		ShaderResourceManager::UpdateDeviceMtlPropBuffer();

		StageExecuteInfo sStageInfo;
		sStageInfo.vecRenderTarget = sInOutFrameInfo.vecRenderTargets;
		sStageInfo.nDepthBufferID = m_vecDepthBuffer[sInOutFrameInfo.nFightFrameIndex]->GetImageID();
		//sStageInfo.vecRenderEntity = sInOutFrameInfo.vecEntityRender;
		sStageInfo.pRenderView = sInOutFrameInfo.pRenderView;
		sStageInfo.nFlightFrameIndex = sInOutFrameInfo.nFightFrameIndex;
		sStageInfo.nWidth = m_nWidth;
		sStageInfo.nHeight = m_nHeight;

		if (sInOutFrameInfo.semAcquiring)
		{
			sStageInfo.vecSemWait.push_back(sInOutFrameInfo.semAcquiring);
			sStageInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		}

		if (sInOutFrameInfo.semDrawing)
		{
			sStageInfo.vecSemSignal.push_back(sInOutFrameInfo.semDrawing);
		}
		
		// 遍历RenderEntity
		// 按照Material分类
		for (EntityRender* pEntity : sInOutFrameInfo.vecEntityRender) {
			sStageInfo.mapRenderEntity[pEntity->GetMaterialID()].push_back(pEntity);
		}

		m_sStage.Execute(sStageInfo);

		sInOutFrameInfo.fenceDrawing = sStageInfo.refFence;
	}

} //namespace LT