#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "Engine.h"
#include "Renderer.h"

#include "View.hpp"

// -- SceneNode --
#include "SceneManager.hpp"
#include "NodeMesh.hpp"

#include "DeviceMemoryManager.h"
#include "BufferManager.h"
#include "MeshManager.hpp"


#include "ImageManager.h"

#include "SamplerManager.h"


#include "MaterialManager.hpp"

#include "EntityManager.hpp"

// -- Image Process --
#include "LunarImageIO.hpp"

// -- Component --
#include "CompSprite3D.hpp"


namespace LT {

	Renderer* Engine::s_pDefaultRenderer = nullptr;

	int64_t Engine::GenCameraID()
	{
		return m_nCameraCounter++;
	}

	Engine::Engine()
		:m_pDebugRenderer(nullptr)
		, m_nFrameIndex(0)
		, m_nWidth(1280)
		, m_nHeight(720)
		, m_bRenderingPaused(false)
		, m_nCameraCounter(0)
		, m_nCameraID(-1)
		, m_nMainScene(-1)
		, m_pView(nullptr)
	{

	}

	Engine::~Engine() {

	}
	vk::Instance Engine::CreateVulkanInstance(const std::vector<const char*>& extensions)
	{
		vkContext::InitVulkanInstance(extensions);
		return vkContext::GetVulkanInstance();
	}
	void Engine::InitVulkanContext(vk::SurfaceKHR surface)
	{
		vkContext::InitVulkanDevice(surface);


		DeviceMemoryManager::Init();
		BufferManager::Init();
		ImageManager::Init();


		SamplerManager::Init();

		MaterialManager::Init();
		MeshManager::Init();

		EntityManager::Init();

		SceneManager::Init();

		s_pDefaultRenderer = new Renderer();
	}
	void Engine::DestroyVulkanContext()
	{
		delete s_pDefaultRenderer;
		s_pDefaultRenderer = nullptr;

		SceneManager::Release();

		EntityManager::Release();

		MeshManager::Release();
		MaterialManager::Release();

		SamplerManager::Release();

		ImageManager::Release();

		BufferManager::Release();

		DeviceMemoryManager::Release();

		vkContext::Release();
	}
	vk::Instance Engine::GetVkInstance()
	{
		return vkContext::GetVulkanInstance();
	}


	void Engine::WaitIdel() {
		vkContext::WaitIdel();
	}

	void Engine::CreateDebugScene(DisplayDevice* pDisplayDivice)
	{
		m_pView = new View();
		CreateCamera(m_nCameraID);
		CreateSceneNode(m_nMainScene, static_cast<int32_t>(NodeType::eNodeMesh));

		m_pView->SetCamera(m_mapCamera[m_nCameraID]);
		m_pView->SetMainScene(m_nMainScene);

		LIIO::ImgRes img("./TestAsset/UVTest.png", 8);
		auto* pImg = ImageManager::CreateImage2DShaderResource(vk::Format::eR8G8B8A8Unorm, img.GetWidth(), img.GetHeight());
		pImg->AssignMemory(img.GetDataPtr(), img.GetDepth() / 8 * img.GetPixelCount() * img.GetChannal());
		m_nImageID = pImg->GetImageID();

		MaterialRef refMtl = MaterialManager::CreateMaterial(MaterialType::eMainTexture);

		MaterialMainTexture* pMtl = dynamic_cast<MaterialMainTexture*>(refMtl.GetPtr());
		pMtl->SetMainTexture(m_nImageID);

		NodeMesh* pNode = dynamic_cast<NodeMesh*>(SceneManager::GetNode(m_nMainScene));
		if (pNode)
		{
			MeshRef cube = MeshManager::CreateCube(1.0f);

			pNode->SetMesh(cube);

			pNode->AddComponent(ComponentType::eSprite3D);
			CompSprite3D* pSprite = dynamic_cast<CompSprite3D*>(pNode->GetComponent(ComponentType::eSprite3D));
			if (pSprite)
			{
				pSprite->SetMesh(cube);
				pSprite->SetMaterial(refMtl);
			}
		}

		pDisplayDivice->SetView(m_pView);
	}

	void Engine::UpdateDebugScene()
	{
		m_nFrameIndex++;

		const float tValue = static_cast<float>(m_nFrameIndex % 200) / 200.0f * glm::pi<float>() * 2.f;

		const float fRadius = 4.f;
		const float fX = fRadius * glm::cos(tValue);
		const float fZ = fRadius * glm::sin(tValue);
		const float fY = 2.f;

		CameraPerspective* pCamera = dynamic_cast<CameraPerspective*>( m_mapCamera[m_nCameraID]);

		if (pCamera)
		{

			pCamera->SetEye(glm::vec3(fX, fY, fZ));
			//glm::mat4 viewMat = m_camera.GetViewMat();

			float fAspect = 1.f;
			if (m_nHeight != 0 && m_nWidth != 0)
			{
				fAspect = static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight);
			}

			pCamera->SetAspect(fAspect);
			pCamera->SetEye(glm::vec3(fX, fY, fZ));

		}
	}

	void Engine::DestroyDebugScene()
	{
#define SAFE_DELETE(ptr) do{if(ptr){delete (ptr); (ptr) = nullptr;}} while(false)

		SAFE_DELETE(m_pView);
		SceneManager::ReleaseNode(m_nMainScene);
		ImageManager::DeleteImage(m_nImageID);
	}

	EngineResult Engine::CreateSceneNode(int64_t& nOutNodeID, int32_t nNodeType)
	{
		nOutNodeID = SceneManager::CreateNode(static_cast<NodeType>(nNodeType));

		if (nOutNodeID == INVALID_NODE_ID)
		{
			return EngineResult::eFailed;
		}
		else
		{
			return EngineResult::eSuccess;
		}
	}

	EngineResult Engine::DeleteSceneNode(int64_t nNodeID)
	{
		SceneManager::ReleaseNode(nNodeID);

		return EngineResult::eSuccess;
	}

	Renderer* Engine::GetDefaultRenderer()
	{
		return s_pDefaultRenderer;
	}

	EngineResult Engine::CreateCamera(int64_t& nOutCameraID)
	{
		nOutCameraID = GenCameraID();
		m_mapCamera[nOutCameraID] = new CameraPerspective();
		return EngineResult::eSuccess;
	}

	EngineResult Engine::DeleteCamera(int64_t nCameraID)
	{
		auto iter = m_mapCamera.find(nCameraID);
		if (iter == m_mapCamera.end())
		{
			delete iter->second;
			m_mapCamera.erase(iter);
			return EngineResult::eSuccess;
		}

		return EngineResult::eFailed;
	}

	EngineResult Engine::CreateDisplaySurface(DisplaySurface* & pOutDisplaySurface, vk::SurfaceKHR surface, uint32_t nWidth, uint32_t nHeight, DisplayDeviceFlag nFlag) {
		pOutDisplaySurface = new DisplaySurface(surface, nWidth, nHeight);

		if (pOutDisplaySurface)
		{
			return EngineResult::eSuccess;
		}
		else
		{
			return EngineResult::eFailed;
		}
	}

	EngineResult Engine::DeleteDisplaySurface(DisplaySurface* pDiplaySurface) {
		if (pDiplaySurface)
		{
			delete pDiplaySurface;
			return EngineResult::eSuccess;
		}

		return EngineResult::eFailed;
	}

} // namespace LT
