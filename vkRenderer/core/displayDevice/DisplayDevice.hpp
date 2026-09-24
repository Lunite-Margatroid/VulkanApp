#pragma once

namespace LT {

	class View;

	using DisplayDeviceFlag = uint32_t;

	class DisplayDevice {
	protected:
		// 目前没有使用
		DisplayDeviceFlag m_flag;
		View* m_pView;

		int64_t m_nFrameIndex;

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
		
		virtual void Pause() = 0;
		virtual void Resume() = 0;

		int64_t GetCurrentFrameIndex() const { return m_nFrameIndex; }
	protected:
		virtual void FrameBegin();
		virtual void FrameEnd();


	};
} // namespace LT
