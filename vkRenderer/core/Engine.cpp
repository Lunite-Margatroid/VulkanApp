#include "vkRendererCommon.h"
#include "Engine.h"
#include "vkContext.h"
#include "Renderer.h"

namespace LT {

	Engine::Engine()
		:m_pDebugRenderer(nullptr)
	{
		
	}

	Engine::~Engine() {
		
	}
	void Engine::InitRenderer(const std::vector<const char*>& extensions, HWND hWnd)
	{
		vkContext::Init(extensions, hWnd);
	}

	void Engine::InitSwapChain() {
		vkContext::InitSwapChain();
		if (m_pDebugRenderer == nullptr) {
			m_pDebugRenderer = new Renderer();
		}
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
		vkContext::ReleaseSwapChain();
	}

	void Engine::WaitIdel() {
		vkContext::WaitIdel();
	}

	void Engine::ResizeSwapChain(unsigned int width, unsigned int height) {
		vkContext::ResizeSwapChain(width, height);
	}

	void Engine::DrawFrame()
	{
		if (m_pDebugRenderer)
		{
			m_pDebugRenderer->DrawFrame();
		}
	}

} // namespace LT
