#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "DisplaySurface.hpp"

namespace LT {
	DisplaySurface::DisplaySurface(vk::SurfaceKHR surface)
		: m_vkSurface(surface)
	{

	}
} // namespace LT
