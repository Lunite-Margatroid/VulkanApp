#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "DisplaySurface.hpp"


namespace LT {
	DisplaySurface::DisplaySurface(vk::SurfaceKHR surface)
		: m_vkSurface(surface) {
	}

	DisplaySurface::~DisplaySurface() {


		
	}


	void DisplaySurface::Pause()
	{
		m_bPause = true;
	}

	void DisplaySurface::Resume()
	{
		m_bPause = false;
	}

	void DisplaySurface::FrameBegin()
	{
		DisplayDevice::FrameBegin();
	}
	void DisplaySurface::FrameEnd()
	{
		DisplayDevice::FrameEnd();
	}
} // namespace LT
