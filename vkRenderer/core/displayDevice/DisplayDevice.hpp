#pragma once
#include "View.hpp"

namespace LT {
	using DisplayDeviceFlag = uint32_t;

	class DisplayDevice {
	private:
		// 目前没有使用
		DisplayDeviceFlag m_flag;

		View* m_pView;

	public:
		DisplayDevice();
		virtual ~DisplayDevice() = default;

		// 三五法则:删除拷贝和移动
		DisplayDevice(const DisplayDevice&) = delete;
		DisplayDevice& operator=(const DisplayDevice&) = delete;
		DisplayDevice(DisplayDevice&&) = delete;
		DisplayDevice& operator=(DisplayDevice&&) = delete;

		void SetView(View* pView) { m_pView = pView; }
		View* GetView() const { return m_pView; }

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Present() = 0;
	};
} // namespace LT
