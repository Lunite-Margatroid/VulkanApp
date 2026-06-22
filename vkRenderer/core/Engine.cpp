#include "vkRendererCommon.h"
#include "Engine.h"
#include "vkContext.h"

namespace LT {

	Engine::Engine()
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
	}

	void Engine::ReleaseRenderer()
	{
		vkContext::Release();
	}

	void Engine::ReleaseSwapChain() {
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
		vkContext::DebugFrame();
	}

} // namespace LT
