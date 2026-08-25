#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "Engine.h"
#include "vkContext.h"
#include "Renderer.h"

namespace LT {

	Engine::Engine()
		:m_pDebugRenderer(nullptr)
		, m_nFrameIndex(0)
		, m_nWidth(1280)
		, m_nHeight(720)
		, m_bRenderingPaused(false)
	{
		
	}

	Engine::~Engine() {
		
	}
	void Engine::InitRenderer(const std::vector<const char*>& extensions, uint32_t nWidth, uint32_t nHeight, void* hWnd)
	{
		vkContext::Init(extensions, nWidth, nHeight, hWnd);

		m_nWidth = nWidth;
		m_nHeight = nHeight;

		InitDebugPipeline();
	}

	void Engine::InitRenderer(vk::Instance vkInstance, vk::SurfaceKHR vkSurface, uint32_t nWidth, uint32_t nHeight)
	{
		vkContext::Init(vkInstance, vkSurface, nWidth, nHeight);
		m_nWidth = nWidth;
		m_nHeight = nHeight;

		InitDebugPipeline();
	}

	void Engine::ReleaseRenderer()
	{
		vkContext::Release();
	}

	void Engine::ReleaseSwapChain() {
		if (m_pDebugRenderer)
		{
			delete m_pDebugRenderer;
			m_pDebugRenderer = nullptr;
		}
		vkContext::GetInstance().ReleaseSwapChain();
	}

	void Engine::WaitIdel() {
		vkContext::WaitIdel();
	}

	void Engine::ResizeSwapChain(unsigned int width, unsigned int height) {
		if (m_bRenderingPaused)
			return;
		m_nWidth = width;
		m_nHeight = height;
		vkContext::ResizeSwapChain(width, height);
		m_pDebugRenderer->Resize(width, height);
	}

	void Engine::DrawFrame()
	{
		if (m_bRenderingPaused)
			return;

		if (m_nWidth == 0 || m_nWidth == 0)
			return;

		m_nFrameIndex++;

		float tValue = static_cast<float>(m_nFrameIndex % 200) / 200.0f * glm::pi<float>() * 2.f;

		float fRadius = 4.f;
		float fX = fRadius * glm::cos(tValue);
		float fZ = fRadius * glm::sin(tValue);
		float fY = 2.f;

		m_camera.SetEye(glm::vec3(fX, fY, fZ));
		//glm::mat4 viewMat = m_camera.GetViewMat();
		
		float fAspect = 1.f;
		if (m_nHeight != 0 && m_nWidth != 0)
		{
			fAspect = static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight);
		}

		float fWidth =( m_camera.GetTop() - m_camera.GetBottom()) * fAspect;

		m_persCamera.SetAspect(fAspect);
		m_persCamera.SetEye(glm::vec3(fX, fY, fZ));

		// m_camera.SetAspect(fAspect);
		m_camera.SetLeft(-fWidth / 2.f);
		m_camera.SetRight(fWidth / 2.f);

		glm::mat4 projectionMat = m_persCamera.GetProjectionMat();
		glm::mat4 viewMat = m_persCamera.GetViewMat();

		m_pDebugRenderer->SetViewMat(reinterpret_cast<float*>(glm::value_ptr(viewMat)));
		m_pDebugRenderer->SetProjectionMat(reinterpret_cast<float*>(glm::value_ptr(projectionMat)));
		m_pDebugRenderer->UpdateConstBufer();

		if (m_pDebugRenderer)
		{
			m_pDebugRenderer->DrawFrame();
		}
	}

	void Engine::PauseRendering()
	{
		m_bRenderingPaused = true;
	}

	void Engine::ResumeRendering()
	{
		m_bRenderingPaused = false;
	}

	void Engine::InitDebugPipeline()
	{
		if (m_pDebugRenderer == nullptr) {
			m_pDebugRenderer = new Renderer();
		}

		m_persCamera.SetAspect(static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight));

		m_camera.SetLeft(-10.f);
		m_camera.SetRight(10.f);
		m_camera.SetTop(10.f);
		m_camera.SetBottom(-10.f);


		glm::mat4 viewMat = m_persCamera.GetViewMat();
		glm::mat4 projectionMat = m_persCamera.GetProjectionMat();
		glm::mat4 modelMat = glm::mat4(1.0f);
		m_pDebugRenderer->SetViewMat(reinterpret_cast<float*>(&viewMat));
		m_pDebugRenderer->SetProjectionMat(reinterpret_cast<float*>(&projectionMat));
		m_pDebugRenderer->SetModelMat(reinterpret_cast<float*>(&modelMat));

		m_pDebugRenderer->UpdateConstBufer();
	}

} // namespace LT
