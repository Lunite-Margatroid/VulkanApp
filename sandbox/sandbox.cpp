#define VK_USE_PLATFORM_WIN32_KHR


// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <iostream>
#include <vector>
#include <optional>
#include <memory>

#include <vulkan/vulkan.hpp>


#include "vkContext.h"


void OnWindowEvent(const SDL_Event& event, SDL_Window* window);

int main() {

    // Create an SDL window that supports Vulkan rendering.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cout << "Could not initialize SDL." << std::endl;
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        std::cout << "Could not create SDL window." << std::endl;
        return 1;
    }

    // Surface Win32Surface SwapChain
    std::vector<const char*> extensions{vk::KHRWin32SurfaceExtensionName,vk::KHRSurfaceExtensionName };


    // Use validation layers if this is a debug build
    std::vector<const char*> layers;
#if defined(_DEBUG)
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    SDL_GetWindowWMInfo(window, &windowInfo);
    // 初始化上下文
    LT::vkContext::Init(extensions, windowInfo.info.win.window);
    // 初始化交换链
    LT::vkContext::InitSwapChain();
    // Poll for user input.
    bool stillRunning = true;
    while (stillRunning) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch (event.type) {

                case SDL_QUIT:
                    stillRunning = false;
                    break;
                case SDL_WINDOWEVENT:
                    OnWindowEvent(event, window);
                    break;

                default:
                    // Do nothing.
                    break;
            }
        }
        LT::vkContext::DebugFrame();
        SDL_Delay(10);
    }
    // 等待空闲
    LT::vkContext::WaitIdel();
    // 释放交换链
    LT::vkContext::ReleaseSwapChain();
    SDL_DestroyWindow(window);
    SDL_Quit();
    // 释放上下问
    LT::vkContext::Release();

	return 0;
}

void OnWindowEvent(const SDL_Event& event, SDL_Window* window) {
    static int nLastWidth, nLastHeight;
    int width, height;
    SDL_GetWindowSize(window, &width, &height);


    if (width != nLastWidth || height != nLastHeight) {
        nLastWidth = width;
        nLastHeight = height;
        LT::vkContext::WaitIdel();
        LT::vkContext::ResizeSwapChain(event.window.data1, event.window.data2);
    }


    //std::cout << event.window.windowID << std::endl;
    //std::cout << width << "   " << height << std::endl;
}