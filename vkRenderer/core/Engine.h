#pragma once
#include "CameraPerspective.h"
#include "CameraOrtho.h"
namespace LT {
	class Renderer;

	class Engine {
	private:
		Renderer* m_pDebugRenderer;
		CameraOrtho m_camera;
		CameraPerspective m_persCamera;


		uint64_t m_nFrameIndex;

		unsigned int m_nWidth;
		unsigned int m_nHeight;

		bool m_bRenderingPaused;



	public:
		Engine();
		~Engine();

		void InitRenderer(const std::vector<const char*>& extensions, uint32_t nWidth, uint32_t nHeight, void* hWnd);
		void InitRenderer(vk::Instance vkInstance, vk::SurfaceKHR vkSurface, uint32_t nWidth, uint32_t nHeight);
		void InitSwapChain(uint32_t nWidth, uint32_t nHeight, void* hWnd);

		void ReleaseRenderer();
		void ReleaseSwapChain();

		void WaitIdel();

		void ResizeSwapChain(unsigned int width, unsigned int height);

		void DrawFrame();

		void PauseRendering();
		void ResumeRendering();
		
	private:
		void InitDebugPipeline();
	};
}