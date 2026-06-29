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

	public:
		Engine();
		~Engine();

		void InitRenderer(const std::vector<const char*>& extensions, HWND hWnd = NULL);
		void InitSwapChain();

		void ReleaseRenderer();
		void ReleaseSwapChain();

		void WaitIdel();

		void ResizeSwapChain(unsigned int width, unsigned int height);

		void DrawFrame();
	};
}