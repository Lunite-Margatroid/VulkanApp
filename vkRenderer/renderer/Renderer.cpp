#include "vkRendererCommon.h"
#include "Renderer.h"
#include "vkContext.h"

namespace LT {
	Renderer::Renderer()
	{
	}
	Renderer::~Renderer()
	{
	}

	void Renderer::DrawFrame() {
		m_pipeline.DrawFrame();
	}
} // namespace