#include "vkRendererCommon.h"
#include "vkContext.h"
#include "Renderer.h"

namespace LT {
	Renderer::Renderer()
	{
		m_pPipeline = std::make_unique<Pipeline>();

		float vertBuffer[] = {
			-0.5f, 0.5f,	1.0f, 0.0f, 0.f,
			0.5f, 0.5f,		0.f, 1.f,0.f,
			0.f, -0.5f,		0.f, 0.f, 1.f
		};

		m_pDebugVertexBuffer = std::make_unique<VertexBuffer>(sizeof(vertBuffer), vertBuffer, 3);

		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Position, BufferDataType::TypeFloat32, 2, 0
		));
		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Color, BufferDataType::TypeFloat32, 3, 8
		));

		m_pPipeline->SetVertexBuffer(m_pDebugVertexBuffer.get());
	}
	Renderer::~Renderer()
	{
		m_pDebugVertexBuffer.reset();
		m_pPipeline.reset();
	}

	void Renderer::DrawFrame() {
		m_pPipeline->DrawFrame();
	}
} // namespace