#pragma once
#include "DisplayDevice.hpp"

namespace LT {
	class DisplaySurface : public DisplayDevice {
	protected:
		vk::SurfaceKHR m_vkSurface;

	public:
		DisplaySurface(vk::SurfaceKHR surface);
		virtual ~DisplaySurface() = default;

		// 三五法则:删除拷贝和移动
		DisplaySurface(const DisplaySurface&) = delete;
		DisplaySurface& operator=(const DisplaySurface&) = delete;
		DisplaySurface(DisplaySurface&&) = delete;
		DisplaySurface& operator=(DisplaySurface&&) = delete;

		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;
	};
} // namespace LT
