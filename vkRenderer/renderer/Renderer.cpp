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
		VK_CONTEXT_DEVICE;

		m_pRenderView.reset(new RenderViewSingleCamera());

		for (int i = 0; i < m_arrSemphores.size(); ++i)
		{
			for (int j = 0; j < m_arrSemphores[i].size(); ++j)
			{
				m_arrSemphores[i][j] = device.createSemaphore(vk::SemaphoreCreateInfo({}));
			}
		}
	}
	Renderer::~Renderer()
	{
		VK_CONTEXT_DEVICE;
		for (int i = 0; i < m_arrSemphores.size(); ++i)
		{
			for (int j = 0; j < m_arrSemphores[i].size(); ++j)
			{
				device.destroySemaphore(m_arrSemphores[i][j]);
			}
		}

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

		FlightFrameIndex nFlightFrameIndex = sInOutFrameInfo.nFightFrameIndex;

		// Update RenderView
		Camera* pCamera = m_pView->GetCamera();
		std::array<vk::Semaphore, RENDERER_SEMPHORE_COUNT>& arrSemphore = m_arrSemphores[nFlightFrameIndex];


		m_pRenderView->SetCameraPos(pCamera->GetEye());
		m_pRenderView->SetProjectionMat(pCamera->GetProjectionMat());
		m_pRenderView->SetViewMat(pCamera->GetViewMat());
		

		// 收集RenderEntity
		m_pView->GetRenderEntityAndLight(sInOutFrameInfo.vecEntityRender, sInOutFrameInfo.vecLightBuffer, sInOutFrameInfo.bLightDirty);
		sInOutFrameInfo.pRenderView = m_pRenderView.get();

		const std::vector<Layer*>& layers = m_pView->GetLayers();

		PipelineExecuteInfo sPipelineExecuteInfo = {};
		sPipelineExecuteInfo.nFrameIndex = sInOutFrameInfo.nFrameIndex;
		sPipelineExecuteInfo.nFightFrameIndex = nFlightFrameIndex;
		sPipelineExecuteInfo.vecEntityRender = sInOutFrameInfo.vecEntityRender;
		sPipelineExecuteInfo.pRenderView = sInOutFrameInfo.pRenderView;
		sPipelineExecuteInfo.vecRenderTargets = sInOutFrameInfo.vecRenderTargets;
		sPipelineExecuteInfo.nWidth = sInOutFrameInfo.nWidth;
		sPipelineExecuteInfo.nHeight = sInOutFrameInfo.nHeight;
		sPipelineExecuteInfo.bLightDirty = sInOutFrameInfo.bLightDirty;
		sPipelineExecuteInfo.vecLightBuffer = sInOutFrameInfo.vecLightBuffer;
		sPipelineExecuteInfo.vecColorClear.push_back(true);
		sPipelineExecuteInfo.refFence = sInOutFrameInfo.fenceDrawing;

		bool bFirstPipeline = true;
		uint32_t nPipelineCounter = 0;
		for (int i =0; i < layers.size();++i)
		{
			const std::vector<Pipeline*> pipelines = layers[i]->GetPipelines();

			for (int j = 0; j < pipelines.size(); ++j)
			{
				nPipelineCounter += 1;

				//bool bLastPipeline = (i == layers.size() - 1) && (j == pipelines.size() - 1);

				//sPipelineExecuteInfo.vecSemSignal.clear();
				//if (bLastPipeline)
				//{
				//	sPipelineExecuteInfo.vecSemSignal.push_back(sInOutFrameInfo.semDrawing);
				//}
				//else
				//{
				//	sPipelineExecuteInfo.vecSemSignal.push_back(arrSemphore[nPipelineCounter % RENDERER_SEMPHORE_COUNT]);
				//}

				//if (bFirstPipeline)
				//{
				//	bFirstPipeline = false;
				//	sPipelineExecuteInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllCommands);
				//	sPipelineExecuteInfo.vecSemWait.push_back(sInOutFrameInfo.semAcquiring);
				//}
				//else
				//{
				//	sPipelineExecuteInfo.vecSemWaitMask.clear();
				//	sPipelineExecuteInfo.vecSemWait.clear();

				//	sPipelineExecuteInfo.vecSemWaitMask.push_back(vk::PipelineStageFlagBits::eAllCommands);
				//	sPipelineExecuteInfo.vecSemWait.push_back(arrSemphore[(nPipelineCounter - 1) % RENDERER_SEMPHORE_COUNT]);
				//}

				pipelines[j]->Resize(sInOutFrameInfo.nWidth, sInOutFrameInfo.nHeight);
				pipelines[j]->Execute(sPipelineExecuteInfo);
				if (sPipelineExecuteInfo.refFence)
				{
					for (int i = 0; i < sPipelineExecuteInfo.vecColorClear.size(); ++i)
					{
						sPipelineExecuteInfo.vecColorClear[i] = false;
					}
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