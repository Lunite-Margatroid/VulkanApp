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
		, m_pPipeline(nullptr)
	{
		m_pRenderView.reset(new RenderViewSingleCamera());
		m_pPipeline.reset( new Pipeline());
	}
	Renderer::~Renderer()
	{
		m_pPipeline.reset();
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
		sInOutFrameInfo.vecEntityRender = m_pView->GetRenderEntity();
		sInOutFrameInfo.pRenderView = m_pRenderView.get();

		m_pPipeline->Resize(sInOutFrameInfo.nWidth, sInOutFrameInfo.nHeight);

		m_pPipeline->Execute(sInOutFrameInfo);
	}

	void Renderer::SetDisplayDevice(DisplayDevice* pDisplayDevice) {
		m_pDisplayDevice = pDisplayDevice;
	}

	void Renderer::SetView(View* pView) {
		m_pView = pView;
	}


} // namespace LT