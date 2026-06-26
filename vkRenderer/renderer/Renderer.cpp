#include "vkRendererCommon.h"
#include "vkContext.h"
#include "Renderer.h"
#include "BufferManager.h"
#include "DeviceMemoryManager.h"

namespace LT {
	Renderer::Renderer()
	{
		DeviceMemoryManager::Init();
		BufferManager::Init();

		m_pPipeline = std::make_unique<Pipeline>();

		float vertBuffer[] = {
			-0.5f, 0.5f,	1.0f, 0.0f, 0.f,
			0.5f, 0.5f,		0.f, 1.f,0.f,
			0.f, -0.5f,		0.f, 0.f, 1.f
		};

		m_pDebugVertexBuffer = BufferManager::CreateVertexBuffer(sizeof(vertBuffer), vertBuffer, 3);

		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Position, BufferDataType::TypeFloat32, 2, 0
		));
		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Color, BufferDataType::TypeFloat32, 3, 8
		));

		m_pPipeline->SetVertexBuffer(m_pDebugVertexBuffer);


	}
	Renderer::~Renderer()
	{
		
		BufferManager::DeleteBuffer(m_pDebugVertexBuffer);

		m_pPipeline.reset();

		BufferManager::Release();
		DeviceMemoryManager::Release();
	}

	void Renderer::DrawFrame() {
		m_pPipeline->DrawFrame();
	}
} // namespace