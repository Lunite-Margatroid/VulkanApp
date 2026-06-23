#include "vkRendererCommon.h"
#include "vkContext.h"
#include "Renderer.h"

namespace LT {
	Renderer::Renderer()
	{
		float vertBuffer[] = {
			-0.5f, 0.5f,	1.0f, 0.0f, 0.f,
			0.5f, 0.5f,		0.f, 1.f,0.f,
			0.f, -0.5f,		0.f, 0.f, 1.f
		};

		m_pDebugVertexBuffer = std::make_unique<VertexBuffer>(sizeof(vertBuffer), vertBuffer);

		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Position, BufferDataType::TypeFloat32, 2, 0
		));
		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Color, BufferDataType::TypeFloat32, 3, 8
		));

		m_pipeline.SetVertexBuffer(m_pDebugVertexBuffer->GetNativeBuffer());
	}
	Renderer::~Renderer()
	{
	}

	void Renderer::DrawFrame() {
		m_pipeline.DrawFrame();
	}
} // namespace