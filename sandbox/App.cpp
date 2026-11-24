#include "App.hpp"

#include "vkRendererUtil.hpp"
#include "Timer.hpp"

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>
#include <vector>
#include <optional>
#include <memory>



App::App()
{
	// Create an SDL window that supports Vulkan rendering.
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		LOG_ERROR("Could not initialize SDL.");
		std::terminate();
	}

	m_pEngine = nullptr;
	m_pWindow = nullptr;
	m_vkSurface = VK_NULL_HANDLE;
	m_pDisplaySurface = nullptr;
	m_bEngineRun.store(true);

	SDL_Window* window = SDL_CreateWindow("Vulkan Window", DEFAULT_WIDTH, DEFAULT_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		LOG_ERROR("Could not create SDL window.");
		std::terminate();
		return;
	}
	std::vector<const char*> extensions;

	{
		uint32_t nCount = 0;
		const char* const* vecExt = SDL_Vulkan_GetInstanceExtensions(&nCount);
		for (int i = 0; i < nCount; i++)
		{
			extensions.push_back(vecExt[i]);
		}
	}

	// 初始化上下文
	m_pEngine = new LT::Engine();
	vk::Instance vkInstance = m_pEngine->CreateVulkanInstance(extensions);


	if (!SDL_Vulkan_CreateSurface(window, vkInstance, NULL, reinterpret_cast<VkSurfaceKHR*>(&m_vkSurface)))
	{
		LOG_ERROR("SDL Create Surface Failed. SDL: %s", SDL_GetError());
		std::terminate();
		return;
	}
	m_pEngine->InitVulkanContext(m_vkSurface);

	m_pEngine->CreateDisplaySurface(m_pDisplaySurface, m_vkSurface, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);

	m_pEngine->CreateDebugScene(m_pDisplaySurface);

	m_pShell = std::make_unique<Shell>(m_pEngine);
}

App::~App()
{
	m_pShell.reset();

	SDL_Quit();
}

void App::Run() {
	LT::util::Timer timer;
	double dOneSecond = 1000.0;
	double dDeltaTime = 1.0;
	double dLastTime = timer.GetTick();
	int64_t nFrameCount = 0;

	m_pShell->Execute();

	while (m_bEngineRun.load()) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

				case SDL_EventType::SDL_EVENT_QUIT:
					m_bEngineRun.store(false);
					break;
				default:
					if (event.type & 0x200)
					{
						// 窗口事件
						OnWindowEvent(event, m_pWindow, m_pDisplaySurface, m_pEngine);
					}
					break;
			}
		}
		m_pEngine->UpdateDebugScene();
		m_pDisplaySurface->Present();
		//SDL_Delay(10);
		// Frame Count
		nFrameCount += 1;
		dDeltaTime = timer.GetTick() - dLastTime;
		dLastTime = timer.GetTick();
		dOneSecond += dDeltaTime;
		if (dOneSecond >= 1000.0)
		{
			LOG_INFO("FPS: %lld", nFrameCount);
			nFrameCount = 0;
			dOneSecond -= 1000.0;
		}

		if (!m_pShell->Flush())
		{
			m_bEngineRun.store(false);
		}
	}

	m_pShell->Terminate();
}

void OnWindowEvent(const SDL_Event& event, SDL_Window* window, LT::DisplaySurface* pDisplaySurface, LT::Engine* pEngine) {

	switch (event.type) {
		case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
			pEngine->WaitIdel();
			pDisplaySurface->Resize(event.window.data1, event.window.data2);
			break;
		case SDL_EventType::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		case SDL_EventType::SDL_EVENT_WINDOW_MAXIMIZED:
			pEngine->WaitIdel();
			//pEngine->ResumeRendering();
			//pEngine->ResizeSwapChain(event.window.data1, event.window.data2);
			pDisplaySurface->Resume();
			pDisplaySurface->Resize(event.window.data1, event.window.data2);
			break;
		case SDL_EventType::SDL_EVENT_WINDOW_RESTORED:
			//pEngine->ResumeRendering();
			pDisplaySurface->Resume();
			break;

		case SDL_EventType::SDL_EVENT_WINDOW_MINIMIZED:
			pEngine->WaitIdel();
			//pEngine->PauseRendering();
			pDisplaySurface->Pause();
			break;

		case SDL_EventType::SDL_EVENT_WINDOW_MOVED:
			break;
		default:
			break;
	}

	//const char* strSDL_WindowEventID[] =
	//{
	//    "SDL_WINDOWEVENT_NONE",
	//    "SDL_WINDOWEVENT_SHOWN",
	//    "SDL_WINDOWEVENT_HIDDEN",
	//    "SDL_WINDOWEVENT_EXPOSED",
	//    "SDL_WINDOWEVENT_MOVED",
	//    "SDL_WINDOWEVENT_RESIZED",
	//    "SDL_WINDOWEVENT_SIZE_CHANGED",
	//    "SDL_WINDOWEVENT_MINIMIZED",
	//    "SDL_WINDOWEVENT_MAXIMIZED",
	//    "SDL_WINDOWEVENT_RESTORED",
	//    "SDL_WINDOWEVENT_ENTER",
	//    "SDL_WINDOWEVENT_LEAVE",
	//    "SDL_WINDOWEVENT_FOCUS_GAINED",
	//    "SDL_WINDOWEVENT_FOCUS_LOST",
	//    "SDL_WINDOWEVENT_CLOSE",
	//    "SDL_WINDOWEVENT_TAKE_FOCUS",
	//    "SDL_WINDOWEVENT_HIT_TEST",
	//    "SDL_WINDOWEVENT_ICCPROF_CHANGED",
	//    "SDL_WINDOWEVENT_DISPLAY_CHANGED"
	//};
	//std::cout << "window event: " << strSDL_WindowEventID[static_cast<unsigned int>(event.window.event)] << std::endl;

}