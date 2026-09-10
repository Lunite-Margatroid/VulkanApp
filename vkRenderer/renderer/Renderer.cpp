#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "Renderer.h"
#include "DeviceMemoryManager.h"
#include "ImageManager.h"
#include "image/ImgRes.h"
#include "sampler/SamplerManager.h"
#include "MeshStatic.hpp"

#include "SwapChain.h"

namespace LT {
	Renderer::Renderer()
		:m_nFrameIndex(0)
	{
		DeviceMemoryManager::Init();
		BufferManager::Init();
		ImageManager::Init();
		SamplerManager::Init();
		MaterialManager::Init();
		MeshManager::Init();


		SamplerManager::GetDefaultImageSampler();

		m_pPipeline = std::make_unique<Pipeline>();

		MeshRef refMesh = MeshManager::CreateStaticMesh();
		float arrPosition[] = {
		-0.5f, 0.5f,0.5f,
		0.5f, 0.5f,0.5f,
		0.5f, -0.5f,0.5f,
		-0.5f, -0.5f,0.5f,

		-0.5f, 0.5f,-0.5f,
		0.5f, 0.5f,-0.5f,
		0.5f, -0.5f,-0.5f,
		-0.5f, -0.5f,-0.5f,

		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,-0.5f,  0.5f,
		0.5f,-0.5f,  0.5f,
		0.5f,-0.5f,  -0.5f,
		-0.5f,-0.5f,  -0.5f,


		-0.5f,0.5f,  0.5f,
		0.5f,0.5f,  0.5f,
		0.5f,0.5f,  -0.5f,
		-0.5f,0.5f,  -0.5f,
		};

		float arrUV[] = {
		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,

		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,

		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,

		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,

		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,


		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,
		};

		float arrColor[] = {
			1.0f, 0.0f, 0.f,
			0.f, 1.f,0.f,
			0.f,0.f, 1.f,
			1.f,1.f,1.f,

			1.0f, 0.0f, 0.f,
			0.f, 1.f,0.f,
			0.f, 0.f, 1.f,
			1.f,1.f,1.f,

			1.0f, 0.0f, 0.f,
			0.f, 1.f,0.f,
			0.f, 0.f, 1.f,
			1.f,1.f,1.f,

			1.0f, 0.0f, 0.f,
			0.f, 1.f,0.f,
			0.f, 0.f, 1.f,
			1.f,1.f,1.f,

			1.0f, 0.0f, 0.f,
			0.f, 1.f,0.f,
			0.f, 0.f, 1.f,
			1.f,1.f,1.f,


			1.0f, 0.0f, 0.f,
			0.f, 1.f,0.f,
			0.f, 0.f, 1.f,
			1.f,1.f,1.f,
		};

		// index buffer
		uint32_t indices[36] = {
			0,1,2,2,3,0
		};

		for (int i = 6; i < 36; i++)
		{
			indices[i] = indices[i - 6] + 4;
		}

		refMesh->SetVertexPosition(reinterpret_cast<MeshStatic::SPosition*>(arrPosition), 24);
		refMesh->SetUV(reinterpret_cast<MeshStatic::SUV*>(arrUV), 24, 0);
		refMesh->SetPrimitive(vk::PrimitiveTopology::eTriangleList);

		refMesh->SetIndexBuffer(indices, 36);

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

	void Renderer::DrawFrame() {
		vk::Device& device = vkContext::GetVkDevice();
		vk::SwapchainKHR swapchain = vkContext::GetNativeSwapChain();
		m_nFrameIndex += 1;

		FlightFrameIndex nFlightFrameIndex = m_nFrameIndex % RENDERER_DEFAULT_FLIGHT_FRAME_NUM;

		// 等待同一Flight Frame上一帧绘制
		vk::Result waitResult = device.waitForFences(m_vecFenceDrawing[nFlightFrameIndex], vk::True, std::_Max_limit<uint64_t>());
		RENDERER_ASSERT(waitResult == vk::Result::eSuccess, "Wait for Draing Failed.");
		// 重置
		device.resetFences(m_vecFenceDrawing[nFlightFrameIndex]);

		// 获取Swapchain image
		int32_t imageIndex = vkContext::GetSwapChain().AcquireNextImage(std::_Max_limit<uint64_t>(), m_vecSemAcquiring[nFlightFrameIndex], vk::Fence());
		RENDERER_ASSERT(imageIndex >= 0, "Acquire Swapchain Image Failed.");
		uint32_t nImageIndex = static_cast<uint32_t>(imageIndex);
		
		FrameInfo sFrameInfo;
		sFrameInfo.nFrameIndex = m_nFrameIndex;
		sFrameInfo.nFightFrameIndex = nFlightFrameIndex;
		sFrameInfo.vecRenderTargets.push_back(SWAPCHAIN_IMAGE_ID);
		sFrameInfo.fenceDrawing = m_vecFenceDrawing[nFlightFrameIndex];
		sFrameInfo.semAcquiring = m_vecSemAcquiring[nFlightFrameIndex];
		sFrameInfo.semDrawing = m_vecSemDrawing[nFlightFrameIndex];
		sFrameInfo.vecEntityRender.push_back(m_pEntity.get());
		sFrameInfo.pRenderView = m_pRenderView.get();

		m_pPipeline->Execute(sFrameInfo);

		

		// 交换链命令
		vk::PresentInfoKHR pi;
		pi.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&m_vecSemDrawing[nFlightFrameIndex])	// 等待渲染完成
			.setSwapchainCount(1)
			.setPSwapchains(&swapchain)
			.setPImageIndices(&nImageIndex)
			;
		// 提交交换链命令
		vk::Result resultPresent = vkContext::GetCmdQueueForSurface().presentKHR(pi);

		if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR)
		{
			vkContext::WaitIdel();
			vkContext::GetInstance().ResizeSwapChain(m_pPipeline->GetWidth(), m_pPipeline->GetHeight());
		}
		else
		{
			RENDERER_ASSERT(resultPresent == vk::Result::eSuccess, "Present Failed.");
		}
	}


	void Renderer::Resize(uint32_t nWidth, uint32_t nHeight) {
		m_pPipeline->Resize(nWidth, nHeight);
	}

} // namespace LT