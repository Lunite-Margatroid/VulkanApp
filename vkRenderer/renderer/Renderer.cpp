#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "Renderer.h"
#include "DeviceMemoryManager.h"
#include "ImageManager.h"
#include "ImgRes.hpp"
#include "sampler/SamplerManager.h"
#include "MeshStatic.hpp"

#include "SwapChain.h"

#include "DisplayDevice.hpp"
#include "View.hpp"

namespace LT {
	Renderer::Renderer()
		: m_pDisplayDevice(nullptr)
		, m_pView(nullptr)
	{
		DeviceMemoryManager::Init();
		BufferManager::Init();
		ImageManager::Init();
		SamplerManager::Init();
		MaterialManager::Init();
		MeshManager::Init();


		SamplerManager::GetDefaultImageSampler();

		m_pPipeline = std::make_unique<Pipeline>();

		MeshRef refMesh = MeshManager::CreateCube(1.0f);

		auto[pDebugVertexBuffer, pDebugIndexBuffer] = BufferManager::CreateVertexIndexBuffer(refMesh);
		m_pDebugVertexBuffer = pDebugVertexBuffer;
		m_pDebugIndexBuffer = pDebugIndexBuffer;
		
		m_refMtl = MaterialManager::CreateMaterial(MaterialType::eMainTexture);


		m_pEntity.reset(new EntityRenderMesh(0));
		m_pEntity->SetMaterial(m_refMtl);
		m_pEntity->SetMesh(refMesh);
		m_pEntity->SetIndexBuffer(m_pDebugIndexBuffer);
		m_pEntity->SetVertexBuffer(m_pDebugVertexBuffer);
		

		m_pRenderView.reset(new RenderViewSingleCamera());

		
		ImgRes img("./TestAsset/UVTest.png", 8);
		m_pDebugImage = ImageManager::CreateImage2DShaderResource(vk::Format::eR8G8B8A8Srgb, img.GetWidth(), img.GetHeight());
		m_pDebugImage->AssignMemory(img.GetDataPtr(), img.GetDepth() / 8 * img.GetHeight() * img.GetWidth() * img.GetChannal());

		MaterialMainTexture* pMtl = dynamic_cast<MaterialMainTexture*>(m_refMtl.GetPtr());
		if (pMtl)
		{
			pMtl->SetMainTexture(m_pDebugImage->GetImageID());
		}

		vk::Device& device = vkContext::GetVkDevice();

		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; i++)
		{
			m_vecSemAcquiring.emplace_back(device.createSemaphore(vk::SemaphoreCreateInfo{}));
			m_vecSemDrawing.emplace_back(device.createSemaphore(vk::SemaphoreCreateInfo{}));
			m_vecFenceDrawing.emplace_back(device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
		}
	}
	Renderer::~Renderer()
	{
		m_refMtl.Release();
		m_pEntity.reset();

		vk::Device& device = vkContext::GetVkDevice();

		for (int i = 0; i < RENDERER_DEFAULT_FLIGHT_FRAME_NUM; ++i)
		{
			device.destroySemaphore(m_vecSemAcquiring[i]);
			device.destroySemaphore(m_vecSemDrawing[i]);
			device.destroyFence(m_vecFenceDrawing[i]);
		}


		m_pRenderView.reset();

		ImageManager::DeleteImage(m_pDebugImage);


		BufferManager::DeleteBuffer(reinterpret_cast<Buffer*>(m_pDebugVertexBuffer));
		BufferManager::DeleteBuffer(reinterpret_cast<Buffer*>(m_pDebugIndexBuffer));

		m_pPipeline.reset();


		MeshManager::Release();
		MaterialManager::Release();
		SamplerManager::Release();
		ImageManager::Release();
		BufferManager::Release();
		DeviceMemoryManager::Release();
	}

	void Renderer::SetModelMat(const glm::mat4& matModel)
	{
		m_pRenderView->SetModelMat(matModel);
	}

	void Renderer::SetViewMat(const glm::mat4& matView)
	{
		m_pRenderView->SetViewMat(matView);
	}

	void Renderer::SetProjectionMat(const glm::mat4&  matProjection)
	{
		m_pRenderView->SetProjectionMat(matProjection);
	}

	void Renderer::SetCameraPos(const glm::vec4& vec4CameraPos) {
		m_pRenderView->SetCameraPos(vec4CameraPos);
	}

	void Renderer::DrawFrame(const FrameInfo& sFrameInfo) {
		vk::Device& device = vkContext::GetVkDevice();		

		FrameInfo sPipelineFrameInfo = sFrameInfo;

		// 收集RenderEntity
		sPipelineFrameInfo.vecEntityRender = m_pView->GetRenderEntity();
		sPipelineFrameInfo.pRenderView = m_pRenderView.get();

		m_pPipeline->Execute(sPipelineFrameInfo);
	}


	void Renderer::Resize(uint32_t nWidth, uint32_t nHeight) {
		m_pPipeline->Resize(nWidth, nHeight);
	}

} // namespace LT