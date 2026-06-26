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


		// vertex buffer
		float vertBuffer[] = {
			-0.5f, 0.5f,	1.0f, 0.0f, 0.f,
			0.5f, 0.5f,		0.f, 1.f,0.f,
			0.5f, -0.5f,		0.f, 0.f, 1.f,
			-0.5f, -0.5f,		1.f,1.f,1.f
		};

		m_pDebugVertexBuffer = BufferManager::CreateVertexBuffer(sizeof(vertBuffer), vertBuffer, 4);

		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Position, BufferDataType::TypeFloat32, 2, 0
		));
		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Color, BufferDataType::TypeFloat32, 3, 8
		));

		m_pPipeline->SetVertexBuffer(m_pDebugVertexBuffer);

		// index buffer
		uint32_t indices[] = {
			0,1,2,2,3,0
		};

		m_pDebugIndexBuffer = BufferManager::CreateIndexBuffer(sizeof(indices), indices, 6);

		m_pPipeline->SetIndexBuffer(m_pDebugIndexBuffer);

	}
	Renderer::~Renderer()
	{
		
		BufferManager::DeleteBuffer(m_pDebugVertexBuffer);
		BufferManager::DeleteBuffer(m_pDebugIndexBuffer);

		m_pPipeline.reset();

		BufferManager::Release();
		DeviceMemoryManager::Release();
	}

	void Renderer::DrawFrame() {
		m_pPipeline->DrawFrame();
	}
} // namespace