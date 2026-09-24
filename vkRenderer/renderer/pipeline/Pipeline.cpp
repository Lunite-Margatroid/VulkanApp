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
	}

	Pipeline::~Pipeline() {
		vk::Device& device = vkContext::GetVkDevice();
		RENDERER_ASSERT(m_vecDepthBuffer.size() == RENDERER_DEFAULT_FLIGHT_FRAME_NUM, "func: %s. Depth Buffer Count Error.", __FUNCTION__);
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

	void Pipeline::Execute(const FrameInfo& sFrameInfo)
	{
		RenderStageOpaqueForward stage;

		StageExecuteInfo sStageInfo;
		sStageInfo.vecRenderTarget = sFrameInfo.vecRenderTargets;
		sStageInfo.nDepthBufferID = m_vecDepthBuffer[sFrameInfo.nFightFrameIndex]->GetImageID();
		sStageInfo.vecRenderEntity = sFrameInfo.vecEntityRender;
		sStageInfo.pRenderView = sFrameInfo.pRenderView;
		sStageInfo.nFlightFrameIndex = sFrameInfo.nFightFrameIndex;
		sStageInfo.nWidth = m_nWidth;
		sStageInfo.nHeight = m_nHeight;

		if (sFrameInfo.semAcquiring)
		{
			sStageInfo.vecSemWait.push_back(sFrameInfo.semAcquiring);
			sStageInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		}

		if (sFrameInfo.semDrawing)
		{
			sStageInfo.vecSemSignal.push_back(sFrameInfo.semDrawing);
		}

		if (sFrameInfo.fenceDrawing)
		{
			sStageInfo.fenceSet = sFrameInfo.fenceDrawing;
		}
		
		stage.Execute(sStageInfo);
	}

} //namespace LT