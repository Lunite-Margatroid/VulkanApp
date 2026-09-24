#pragma once
#include "DisplayDevice.hpp"

namespace LT {

	class SwapChain;

	class DisplaySurface : public DisplayDevice {
	protected:
		vk::SurfaceKHR m_vkSurface;
		SwapChain* m_pSwapChain;
		int64_t m_nSwapChainImageID;
		int32_t m_nSwapChainImageIndex;

		uint32_t m_nWidth;
		uint32_t m_nHeight;

		std::vector<vk::Semaphore> m_vecSemDrawing;
		std::vector<vk::Semaphore> m_vecSemAcquiring;
		std::vector<vk::Fence> m_vecFenceDrawing;

		bool m_bPause;

	public:
		DisplaySurface(vk::SurfaceKHR surface, uint32_t nWidth, uint32_t nHeight);
		virtual ~DisplaySurface();

		// 三五法则:删除拷贝和移动
		DisplaySurface(const DisplaySurface&) = delete;
		DisplaySurface& operator=(const DisplaySurface&) = delete;
		DisplaySurface(DisplaySurface&&) = delete;
		DisplaySurface& operator=(DisplaySurface&&) = delete;

		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;

		void Pause() override;
		void Resume() override;

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }


	protected:
		void FrameBegin() override;
		void FrameEnd() override;
	};
} // namespace LT
