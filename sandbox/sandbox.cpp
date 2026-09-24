// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "vkRendererUtil.hpp"

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>
#include <vector>
#include <optional>
#include <memory>

#include "EngineCommon.h"
#include "Engine.h"

#if _WIN32
#include <windows.h>
#endif

void OnWindowEvent(const SDL_Event& event, SDL_Window* window, LT::DisplaySurface* pDisplaySurface, LT::Engine* pEngine);

int main() {

	// Create an SDL window that supports Vulkan rendering.
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		std::cout << "Could not initialize SDL." << std::endl;
		return 1;
	}


	SDL_Window* window = SDL_CreateWindow("Vulkan Window", DEFAULT_WIDTH, DEFAULT_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		return 1;
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
	std::unique_ptr<LT::Engine> pEngine(new LT::Engine());
	vk::Instance vkInstance = pEngine->CreateVulkanInstance(extensions);

	vk::SurfaceKHR vkSurface;
	if (!SDL_Vulkan_CreateSurface(window, vkInstance, NULL, reinterpret_cast<VkSurfaceKHR*>(&vkSurface)))
	{
		std::cout << "SDL Create Surface Failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	pEngine->InitVulkanContext(vkSurface);

	LT::DisplaySurface* pDisplaySurface = nullptr;
	pEngine->CreateDisplaySurface(pDisplaySurface, vkSurface, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);

	pEngine->CreateDebugScene(pDisplaySurface);

	// Poll for user input.
	bool stillRunning = true;
	while (stillRunning) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

				case SDL_EventType::SDL_EVENT_QUIT:
					stillRunning = false;
					break;
				default:
					if (event.type & 0x200)
					{
						// 窗口事件
						OnWindowEvent(event, window, pDisplaySurface, pEngine.get());
					}
					break;
			}
		}
		pEngine->UpdateDebugScene();
		pDisplaySurface->Present();
		SDL_Delay(10);
	}
	// 等待空闲
	pEngine->WaitIdel();

	pEngine->DestroyDebugScene();

	pEngine->DeleteDisplaySurface(pDisplaySurface);

	SDL_Vulkan_DestroySurface(vkInstance, vkSurface, NULL);

	pEngine->DestroyVulkanContext();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
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