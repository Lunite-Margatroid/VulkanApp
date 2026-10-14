#pragma once
#include "DisplaySurface.hpp"
#include "FenceManager.hpp"

namespace LT
{
	class DisplayWindowSurface : public DisplaySurface
	{
	private:
		SwapChain* m_pSwapChain;
		int64_t m_nSwapChainImageID;
		int32_t m_nSwapChainImageIndex;

		uint32_t m_nWidth;
		uint32_t m_nHeight;

		std::vector<vk::Semaphore> m_vecSemDrawing;
		std::vector<vk::Semaphore> m_vecSemAcquiring;

		std::array<FenceRef, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_fences;



	public:
		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	public:
		DisplayWindowSurface(vk::SurfaceKHR surface, uint32_t nWidth, uint32_t nHeight);
		virtual ~DisplayWindowSurface();

		// 三五法则:删除拷贝和移动
		DisplayWindowSurface(const DisplayWindowSurface&) = delete;
		DisplayWindowSurface& operator=(const DisplayWindowSurface&) = delete;
		DisplayWindowSurface(DisplayWindowSurface&&) = delete;
		DisplayWindowSurface& operator=(DisplayWindowSurface&&) = delete;

		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;


	protected:
		void FrameBegin() override;
		void FrameEnd() override;
	};
}