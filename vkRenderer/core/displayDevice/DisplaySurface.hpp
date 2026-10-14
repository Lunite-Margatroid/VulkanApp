#pragma once
#include "DisplayDevice.hpp"

namespace LT {

	class SwapChain;

	class DisplaySurface : public DisplayDevice {
	protected:
		vk::SurfaceKHR m_vkSurface;
		bool m_bPause;

	public:
		DisplaySurface(vk::SurfaceKHR surface);
		virtual ~DisplaySurface();

		// 三五法则:删除拷贝和移动
		DisplaySurface(const DisplaySurface&) = delete;
		DisplaySurface& operator=(const DisplaySurface&) = delete;
		DisplaySurface(DisplaySurface&&) = delete;
		DisplaySurface& operator=(DisplaySurface&&) = delete;

		void Pause() override;
		void Resume() override;


	protected:
		void FrameBegin() override;
		void FrameEnd() override;
	};
} // namespace LT
