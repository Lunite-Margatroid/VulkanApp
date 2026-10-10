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
#include "CompTransform3D.hpp"


namespace LT {
	namespace EngineHelper {
		
	}


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
		, m_nCameraID(INVALID_CAMERA_ID)
		, m_nMainScene(INVALID_NODE_ID)
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

		ShaderResourceManager::Init();

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

		ShaderResourceManager::Release();

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
		CreateSceneNode(m_nMainScene, NodeType::eNodeMesh);

		m_pView->SetCamera(m_mapCamera[m_nCameraID]);
		m_pView->SetMainScene(m_nMainScene);

		LIIO::ImgRes img("./TestAsset/UVTest.png", 8);
		auto* pImg = ImageManager::CreateImage2DShaderResource(vk::Format::eR8G8B8A8Unorm, img.GetWidth(), img.GetHeight());
		pImg->AssignMemory(img.GetDataPtr(), img.GetDepth() / 8 * img.GetPixelCount() * img.GetChannal());
		m_nImageID = pImg->GetImageID();

		MaterialID nMtlID = INVALID_MATERIAL_ID;
		CreateMaterial(nMtlID, MaterialType::eMainTexture);
		SetMaterialProp(nMtlID, MtlProp::eTexDiffuse, m_nImageID);

		for (int i = 0; i < 100; i++)
		{
			NodeID nNode;
			CreateCubeMeshNode(nNode, 1.f);
			SetMaterial(nNode, nMtlID);
			SceneNodeRebase(m_nMainScene, nNode);
			SetSceneNodePosition(nNode, util::RandomGenerator::RandomSampleSphere(20.f));
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

	EngineResult Engine::CreateSceneNode(NodeID& nOutNodeID, NodeType nNodeType)
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

	EngineResult Engine::SceneNodeRebase(NodeID nParient, NodeID nChild)
	{
		Node* pParient = SceneManager::GetNode(nParient);
		Node* pChild = SceneManager::GetNode(nChild);

		if (!pParient || !pChild)
		{
			return EngineResult::eNoSceneNode;
		}

		if (Node* pTemp = pChild->GetParent())
		{
			pTemp->RemoveChild(pChild);
		}

		pParient->AddChild(pChild);

		return EngineResult::eSuccess;
	}

	EngineResult Engine::DeleteSceneNode(NodeID nNodeID)
	{
		SceneManager::ReleaseNode(nNodeID);

		return EngineResult::eSuccess;
	}

	EngineResult Engine::SetSceneNodePosition(NodeID nNodeID, const std::array<float, 3>& arrPosition)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (pNode)
		{
			if (auto* pTransform = pNode->GetComponent<CompTransform3D>())
			{
				pTransform->SetPosition(glm::vec3(arrPosition[0], arrPosition[1], arrPosition[2]));
				return EngineResult::eSuccess;
			}
			else
			{
				return EngineResult::eNoComponent;
			}
		}
		else
			return EngineResult::eNoSceneNode;
	}

	EngineResult Engine::SetSceneNodeEulerRotation(NodeID nNodeID, const std::array<float, 3>& arrEulerRotation)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (pNode)
		{
			if (auto* pTransform = pNode->GetComponent<CompTransform3D>())
			{
				pTransform->SetEulerRotation(glm::vec3(arrEulerRotation[0], arrEulerRotation[1], arrEulerRotation[2]));
				return EngineResult::eSuccess;
			}
			else
			{
				return EngineResult::eNoComponent;
			}
		}
		else
			return EngineResult::eNoSceneNode;
	}

	EngineResult Engine::SetSceneNodeScale(NodeID nNodeID, const std::array<float, 3>& arrScale)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (pNode)
		{
			if (auto* pTransform = pNode->GetComponent<CompTransform3D>())
			{
				pTransform->SetScale(glm::vec3(arrScale[0], arrScale[1], arrScale[2]));
				return EngineResult::eSuccess;
			}
			else
			{
				return EngineResult::eNoComponent;
			}
		}
		else
			return EngineResult::eNoSceneNode;
	}

	EngineResult Engine::AddComponent(NodeID nNodeID, ComponentType eCompType)
	{
		if (Node* pNode = SceneManager::GetNode(nNodeID)) {
			pNode->AddComponent(eCompType);
			if (pNode->GetComponent(eCompType))
			{
				return EngineResult::eSuccess;
			}
			else
			{
				return EngineResult::eInvalidParam;
			}
		}
		else
			return EngineResult::eNoSceneNode;
	}

	Renderer* Engine::GetDefaultRenderer()
	{
		return s_pDefaultRenderer;
	}

	EngineResult Engine::CreateCamera(CameraID& nOutCameraID)
	{
		nOutCameraID = GenCameraID();
		m_mapCamera[nOutCameraID] = new CameraPerspective();
		return EngineResult::eSuccess;
	}

	EngineResult Engine::DeleteCamera(CameraID nCameraID)
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

	EngineResult Engine::CreateCubeMeshNode(NodeID& nOutNode, float fSideLength)
	{
		nOutNode = SceneManager::CreateNode(NodeType::eNodeMesh);
		NodeMesh* pNode = reinterpret_cast<NodeMesh*>(SceneManager::GetNode(nOutNode));
		pNode->SetMesh(MeshManager::CreateCube(fSideLength));
		return EngineResult::eSuccess;
	}

	EngineResult Engine::CreateSphereMeshNode(NodeID& nOutNode, float fRadius, uint32_t nLongSubdivision, uint32_t nLatSubdivision)
	{
		nOutNode = SceneManager::CreateNode(NodeType::eNodeMesh);
		NodeMesh* pNode = reinterpret_cast<NodeMesh*>(SceneManager::GetNode(nOutNode));
		pNode->SetMesh(MeshManager::CreateSphere(fRadius, nLongSubdivision, nLatSubdivision));
		return EngineResult::eSuccess;
	}

	EngineResult Engine::CreateMaterial(MaterialID& nOutMtlID, MaterialType eMtlType)
	{
		MaterialRef refMtl = MaterialManager::CreateMaterial(eMtlType);
		refMtl.AddRef();
		nOutMtlID = refMtl.GetID();
		MaterialManager::GetInstance().RefIncrease(nOutMtlID);
		return EngineResult::eSuccess;
	}

	EngineResult Engine::DeleteMaterial(MaterialID nMtlID)
	{
		if (MaterialManager::GetMaterial(nMtlID))
		{
			MaterialManager::GetInstance().RefDecrease(nMtlID);

			return EngineResult::eSuccess;
		}
		else
			return EngineResult::eNoMaterial;

	}

	EngineResult Engine::SetMaterialProp(MaterialID nMtlID, MtlProp eProp, const MtlPropVar& vVar)
	{
		if (IMaterial* pMtl = MaterialManager::GetMaterial(nMtlID))
		{
			return pMtl->SetMtlProp(eProp, vVar);
		}
		else
			return EngineResult::eNoMaterial;
	}

	EngineResult Engine::SetMaterial(NodeID nNodeID, MaterialID nMtlID)
	{
		if (Node* pNode = SceneManager::GetNode(nNodeID))
		{
			if (auto* pSprite = pNode->GetComponent<CompSprite3D>()) {
				pSprite->SetMaterial(MaterialRef(nMtlID));
				return EngineResult::eSuccess;
			}
			else
				return EngineResult::eNoComponent;
		}
		else
			return EngineResult::eNoSceneNode;
	}

} // namespace LT
