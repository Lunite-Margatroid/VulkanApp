#pragma once

namespace LT {
	class Renderer;

	class Engine {
	private:
		Renderer* m_pDebugRenderer;
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