#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "Renderer.h"
#include "DeviceMemoryManager.h"
#include "ImageManager.h"
#include "ImgRes.hpp"
#include "sampler/SamplerManager.h"
#include "MeshStatic.hpp"

#include "SwapChain.h"

#include "DisplayDevice.hpp"
#include "View.hpp"

#include "Camera.h"
#include "CameraPerspective.h"

namespace LT {
	Renderer::Renderer()
		: m_pDisplayDevice(nullptr)
		, m_pView(nullptr)
	{
		m_pRenderView.reset(new RenderViewSingleCamera());
	}
	Renderer::~Renderer()
	{
		m_pRenderView.reset();
	}

	void Renderer::SetViewMat(const glm::mat4& matView)
	{
		m_pRenderView->SetViewMat(matView);
	}

	void Renderer::SetProjectionMat(const glm::mat4&  matProjection)
	{
		m_pRenderView->SetProjectionMat(matProjection);
	}

	void Renderer::SetCameraPos(const glm::vec4& vec4CameraPos) {
		m_pRenderView->SetCameraPos(vec4CameraPos);
	}

	void Renderer::DrawFrame(FrameInfo& sInOutFrameInfo) {
		vk::Device& device = vkContext::GetVkDevice();		

		// Update RenderView
		Camera* pCamera = m_pView->GetCamera();

		m_pRenderView->SetCameraPos(pCamera->GetEye());
		m_pRenderView->SetProjectionMat(pCamera->GetProjectionMat());
		m_pRenderView->SetViewMat(pCamera->GetViewMat());
		

		// 收集RenderEntity
		m_pView->GetRenderEntityAndLight(sInOutFrameInfo.vecEntityRender, sInOutFrameInfo.vecLightBuffer, sInOutFrameInfo.bLightDirty);
		sInOutFrameInfo.pRenderView = m_pRenderView.get();

		const std::vector<Layer*>& layers = m_pView->GetLayers();

		PipelineExecuteInfo sPipelineExecuteInfo = {};
		sPipelineExecuteInfo.nFrameIndex = sInOutFrameInfo.nFrameIndex;
		sPipelineExecuteInfo.nFightFrameIndex = sInOutFrameInfo.nFightFrameIndex;
		sPipelineExecuteInfo.vecEntityRender = sInOutFrameInfo.vecEntityRender;
		sPipelineExecuteInfo.pRenderView = sInOutFrameInfo.pRenderView;
		sPipelineExecuteInfo.vecRenderTargets = sInOutFrameInfo.vecRenderTargets;
		sPipelineExecuteInfo.vecSemSignal.push_back(sInOutFrameInfo.semDrawing);
		sPipelineExecuteInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllCommands);
		sPipelineExecuteInfo.vecSemWait.push_back(sInOutFrameInfo.semAcquiring);
		sPipelineExecuteInfo.nWidth = sInOutFrameInfo.nWidth;
		sPipelineExecuteInfo.nHeight = sInOutFrameInfo.nHeight;
		sPipelineExecuteInfo.bLightDirty = sInOutFrameInfo.bLightDirty;
		sPipelineExecuteInfo.vecLightBuffer = sInOutFrameInfo.vecLightBuffer;
		sPipelineExecuteInfo.vecColorClear.push_back(true);

		for (Layer* pLayer : layers)
		{
			const std::vector<Pipeline*> pipelines = pLayer->GetPipelines();
			for (Pipeline* pPipeline : pipelines)
			{
				pPipeline->Resize(sInOutFrameInfo.nWidth, sInOutFrameInfo.nHeight);
				pPipeline->Execute(sPipelineExecuteInfo);

				for (int i = 0; i < sPipelineExecuteInfo.vecColorClear.size(); ++i)
				{
					sPipelineExecuteInfo.vecColorClear[i] = false;
				}
			}
		}

		sInOutFrameInfo.fenceDrawing = sPipelineExecuteInfo.refFence;
	}

	void Renderer::SetDisplayDevice(DisplayDevice* pDisplayDevice) {
		m_pDisplayDevice = pDisplayDevice;
	}

	void Renderer::SetView(View* pView) {
		m_pView = pView;
	}


} // namespace LT