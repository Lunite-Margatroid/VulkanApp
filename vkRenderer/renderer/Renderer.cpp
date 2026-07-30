#include "vkRendererCommon.h"
#include "vkContext.h"
#include "EngineCommon.h"
#include "Renderer.h"
#include "BufferManager.h"
#include "DeviceMemoryManager.h"
#include "ImageManager.h"
#include "image/ImgRes.h"
#include "sampler/SamplerManager.h"

namespace LT {
	Renderer::Renderer()
	{
		DeviceMemoryManager::Init();
		BufferManager::Init();
		ImageManager::Init();
		SamplerManager::Init();

		SamplerManager::GetDefaultImageSampler();

		m_pPipeline = std::make_unique<Pipeline>();


		// vertex buffer
		float vertBuffer[] = {
			-0.5f, 0.5f,0.5f,	1.0f, 0.0f, 0.f,		0.f, 0.f,
			0.5f, 0.5f,0.5f,		0.f, 1.f,0.f,			1.f, 0.f,
			0.5f, -0.5f,0.5f,		0.f, 0.f, 1.f,		1.f, 1.f,
			-0.5f, -0.5f,0.5f,		1.f,1.f,1.f,		0.f, 1.f,

			-0.5f, 0.5f,-0.5f,		1.0f, 0.0f, 0.f,		0.f, 0.f,
			0.5f, 0.5f,-0.5f,		0.f, 1.f,0.f,			1.f, 0.f,
			0.5f, -0.5f,-0.5f,		0.f, 0.f, 1.f,		1.f, 1.f,
			-0.5f, -0.5f,-0.5f,		1.f,1.f,1.f,		0.f, 1.f,

			0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.f,		0.f, 0.f,
			0.5f, 0.5f, 0.5f,		0.f, 1.f,0.f,			1.f, 0.f,
			0.5f, 0.5f, -0.5f,		0.f, 0.f, 1.f,		1.f, 1.f,
			0.5f, -0.5f, -0.5f,		1.f,1.f,1.f,		0.f, 1.f,

			-0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.f,		0.f, 0.f,
			-0.5f, 0.5f, 0.5f,		0.f, 1.f,0.f,			1.f, 0.f,
			-0.5f, 0.5f, -0.5f,		0.f, 0.f, 1.f,		1.f, 1.f,
			-0.5f, -0.5f, -0.5f,		1.f,1.f,1.f,		0.f, 1.f,

			-0.5f,-0.5f,  0.5f,		1.0f, 0.0f, 0.f,		0.f, 0.f,
			0.5f,-0.5f,  0.5f,		0.f, 1.f,0.f,			1.f, 0.f,
			0.5f,-0.5f,  -0.5f,		0.f, 0.f, 1.f,		1.f, 1.f,
			-0.5f,-0.5f,  -0.5f,		1.f,1.f,1.f,		0.f, 1.f,


			-0.5f,0.5f,  0.5f,		1.0f, 0.0f, 0.f,		0.f, 0.f,
			0.5f,0.5f,  0.5f,		0.f, 1.f,0.f,			1.f, 0.f,
			0.5f,0.5f,  -0.5f,		0.f, 0.f, 1.f,		1.f, 1.f,
			-0.5f,0.5f,  -0.5f,		1.f,1.f,1.f,		0.f, 1.f,
		};

		m_pDebugVertexBuffer = BufferManager::CreateVertexBuffer(sizeof(vertBuffer), vertBuffer, 24);

		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Position, BufferDataType::TypeFloat32, 3, 0
		));
		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::Color, BufferDataType::TypeFloat32, 3, 12
		));
		m_pDebugVertexBuffer->AddVertexChannel(VertexChannelDesc(
			VertexChannel::UV, BufferDataType::TypeFloat32, 2, 24
		));

		m_pPipeline->SetVertexBuffer(m_pDebugVertexBuffer);

		// index buffer
		uint32_t indices[36] = {
			0,1,2,2,3,0
		};

		for (int i = 6; i < 36; i++)
		{
			indices[i] = indices[i - 6] + 4;
		}

		m_pDebugIndexBuffer = BufferManager::CreateIndexBuffer(sizeof(indices), indices, 36);

		m_pPipeline->SetIndexBuffer(m_pDebugIndexBuffer);


		
		// MVP mat
		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++)
		{
			m_vecConstBufferMVPMat.push_back(BufferManager::CreateConstBuffer(sizeof(m_MVPMatBuf), &m_MVPMatBuf));
		}

		m_pPipeline->SetConstBufferMVPMat(m_vecConstBufferMVPMat);

		
		ImgRes img("./TestAsset/UVTest.png", 8);
		m_pDebugImage = ImageManager::CreateImage2DShaderResource(vk::Format::eR8G8B8A8Srgb, img.GetWidth(), img.GetHeight());
		m_pDebugImage->AssignMemory(img.GetDataPtr(), img.GetDepth() / 8 * img.GetHeight() * img.GetWidth() * img.GetChannal());

		m_pPipeline->SetImage(m_pDebugImage);
		m_pPipeline->SetImageSampler(SamplerManager::GetDefaultImageSampler());
		m_pPipeline->UpdateDescriptorSets();
	}
	Renderer::~Renderer()
	{

		ImageManager::DeleteImage(m_pDebugImage);

		for (ConstBuffer* pConstBufer : m_vecConstBufferMVPMat)
		{
			BufferManager::DeleteBuffer(reinterpret_cast<Buffer*>(pConstBufer));
		}
		m_vecConstBufferMVPMat.clear();

		BufferManager::DeleteBuffer(reinterpret_cast<Buffer*>(m_pDebugVertexBuffer));
		BufferManager::DeleteBuffer(reinterpret_cast<Buffer*>(m_pDebugIndexBuffer));

		m_pPipeline.reset();


		SamplerManager::Release();
		ImageManager::Release();
		BufferManager::Release();
		DeviceMemoryManager::Release();
	}

	void Renderer::UpdateConstBufer()
	{
		for (ConstBuffer* pConstBuffer : m_vecConstBufferMVPMat)
		{
			pConstBuffer->UpdateConstBuffer(&m_MVPMatBuf);
		}
	}

	void Renderer::SetModelMat(const float* pModelMat)
	{
		memcpy(&m_MVPMatBuf.modelMat, pModelMat, sizeof(glm::mat4));
	}

	void Renderer::SetViewMat(const float* pViewMat)
	{
		memcpy(&m_MVPMatBuf.viewMat, pViewMat, sizeof(glm::mat4));
	}

	void Renderer::SetProjectionMat(const float* pProjectionMat)
	{
		memcpy(&m_MVPMatBuf.projectionMat, pProjectionMat, sizeof(glm::mat4));
	}

	void Renderer::DrawFrame() {
		m_pPipeline->DrawFrame();
	}

	void Renderer::Resize(uint32_t nWidth, uint32_t nHeight) {
		m_pPipeline->Resize(nWidth, nHeight);
	}

} // namespace LT