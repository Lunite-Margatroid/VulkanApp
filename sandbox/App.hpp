#pragma once
// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "Shell.hpp"

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include "EngineCommon.h"
#include "Engine.h"

#include "vkRendererUtil.hpp"
#include "Timer.hpp"

#if _WIN32
#include <windows.h>
#endif

class App {
private:
	SDL_Window* m_pWindow;
	LT::Engine* m_pEngine;
	vk::SurfaceKHR m_vkSurface;
	LT::DisplaySurface* m_pDisplaySurface;

	std::atomic<bool> m_bEngineRun;

	std::unique_ptr<Shell> m_pShell;

public:
	App();

	~App();

	void Run();
};

void OnWindowEvent(const SDL_Event& event, SDL_Window* window, LT::DisplaySurface* pDisplaySurface, LT::Engine* pEngine);