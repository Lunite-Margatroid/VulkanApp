#include <algorithm>
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "Engine.h"
#include "Renderer.h"

#include "View.hpp"

// -- SceneNode --
#include "SceneManager.hpp"
#include "NodeMesh.hpp"

// -- Resource Managers --
#include "DeviceMemoryManager.h"
#include "BufferManager.h"
#include "MeshManager.hpp"


#include "ImageManager.h"

#include "SamplerManager.h"


#include "MaterialManager.hpp"

#include "EntityManager.hpp"

#include "FenceManager.hpp"

// -- Image Process --
#include "LunarImageIO.hpp"

// -- Component --
#include "CompSprite3D.hpp"
#include "CompTransform3D.hpp"

// -- Light Node --
#include "NodeLight.hpp"
#include "NodePointLight.hpp"
#include "NodeSphereLight.hpp"
#include "NodeSpotLight.hpp"
#include "NodeRectLight.hpp"

// -- Display Window --
#include "DisplayWindowSurface.hpp"
// ---- uitl 计时器 ----
#include "Timer.hpp"

namespace LT {
	namespace EngineHelper {

		// LightType -> NodeType 映射
		NodeType LightTypeToNodeType(LightType eLightType) {
			switch (eLightType) {
				case LightType::eLightPoint:
					return NodeType::eNodePointLight;
				case LightType::eLightSphere:
					return NodeType::eNodeSphereLight;
				case LightType::eLightSpot:
					return NodeType::eNodeSpotLight;
				case LightType::eLightRect:
					return NodeType::eNodeRectLight;
				case LightType::eLightDirectional:
					return NodeType::eNodeDirectionalLight;
				case LightType::eLightAmbient:
					return NodeType::eNodeAmbient;
				default:
					return NodeType::eUnknown;
			}
		}

		// 判断节点是否为灯光节点 是则返回NodeLight*
		// 通过NodeType和LightType判断类型 不使用dynamic_cast
		NodeLight* GetNodeLight(Node* pNode) {
			if (!pNode)
				return nullptr;

			switch (pNode->GetNodeType()) {
				case NodeType::eNodePointLight:
				case NodeType::eNodeSphereLight:
				case NodeType::eNodeSpotLight:
				case NodeType::eNodeRectLight:
				case NodeType::eNodeDirectionalLight:
				case NodeType::eNodeAmbient:
				{
					NodeLight* pLightNode = static_cast<NodeLight*>(pNode);
					// NodeType与LightType相互印证
					RENDERER_ASSERT(LightTypeToNodeType(pLightNode->GetLightType()) == pNode->GetNodeType(), "节点类型与灯光类型不一致.");
					return pLightNode;
				}
				default:
					return nullptr;
			}
		}

		// 判断节点是否为指定类型的灯光节点 是则返回对应类型的指针
		// 通过NodeType和LightType判断类型 不使用dynamic_cast
		template<typename TLightNode>
		TLightNode* GetLightNodeChecked(Node* pNode, LightType eExpectedLightType) {
			if (!pNode || LightTypeToNodeType(eExpectedLightType) != pNode->GetNodeType())
				return nullptr;

			TLightNode* pLightNode = static_cast<TLightNode*>(pNode);
			// NodeType与LightType相互印证
			RENDERER_ASSERT(pLightNode->GetLightType() == eExpectedLightType, "节点类型与灯光类型不一致.");
			return pLightNode;
		}

		// 标记灯光节点及其所在节点树的根节点灯光数据有更新
		// View收集灯光时以根节点的eLightDirty标志为准
		void MarkLightDirty(NodeID nNodeID) {
			Node* pNode = SceneManager::GetNode(nNodeID);
			RENDERER_ASSERT(pNode, "MarkLightDirty: 节点不存在.");
			Node* pRoot = SceneManager::GetNode(pNode->GetRootID());
			RENDERER_ASSERT(pRoot, "MarkLightDirty: 根节点不存在.");

			pNode->SetNodeFlag(static_cast<uint32_t>(pNode->GetNodeFlag()) | static_cast<uint32_t>(NodeFlagBits::eLightDirty));
			pRoot->SetNodeFlag(static_cast<uint32_t>(pRoot->GetNodeFlag()) | static_cast<uint32_t>(NodeFlagBits::eLightDirty));
		}
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

		FenceManager::Init();

		// CommandBufferWrapper用到了FenceRef
		// 初始化要在FenceManager之后
		vkContext::InitCommandBuffers();

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

		vkContext::ReleaseCommandBuffers();

		FenceManager::Release();

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
#define ENGINE_CALL_CHECK(x) do{EngineResult nResult = (x); if(nResult != EngineResult::eSuccess){ LOG_WARNING("Engine call failed"); }} while(false)

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
		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eMainTexture));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexDiffuse, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eExample));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexDiffuse, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eExample));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexSpecular, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eExample));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexIOR, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eExample));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexMeltalness, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eExample));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexRoughness, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eExample));
		//ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexAlpha, m_nImageID));
		//m_vecMtlID.push_back(nMtlID);

		ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::ePhong));
		ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eTexDiffuse, m_nImageID));
		ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eColorDiffuse, std::array<float, 3>{1.f, 1.f, 1.f}));
		ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eRoughness, 0.5f));
		ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eIOR, 1.5f));
		ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eMetalness, 0.f));
		ENGINE_CALL_CHECK(SetMaterialProp(nMtlID, MtlProp::eColorSpecular, std::array<float, 3>{1.f, 1.f, 1.f}));
		m_vecMtlID.push_back(nMtlID);


		//ENGINE_CALL_CHECK(CreateMaterial(nMtlID, MaterialType::eError));
		//m_vecMtlID.push_back(nMtlID);

		// 灯光
		{
			NodeID nLightNode = INVALID_NODE_ID;
			// 环境光
			//ENGINE_CALL_CHECK(CreateLightNode(nLightNode, m_nMainScene, LightType::eLightAmbient));
			//ENGINE_CALL_CHECK(SetLightColor(nLightNode, { 0.1f, 0.1f, 0.1f }));

			// 平行光
			ENGINE_CALL_CHECK(CreateLightNode(nLightNode, m_nMainScene, LightType::eLightDirectional));
			ENGINE_CALL_CHECK(SetLightColor(nLightNode, { 1.f, 1.f, 1.f }));

			// 点光源
			//ENGINE_CALL_CHECK(CreateLightNode(nLightNode, m_nMainScene, LightType::eLightPoint));
			//ENGINE_CALL_CHECK(SetLightColor(nLightNode, { 1.f, 0.9f, 0.7f }));
			//ENGINE_CALL_CHECK(SetLightRange(nLightNode, 50.f));
		}

		for (int i = 0; i < 400; i++)
		{
			NodeID nNode;
			ENGINE_CALL_CHECK(CreateCubeMeshNode(nNode, 1.f));
			ENGINE_CALL_CHECK(SetMaterial(nNode, m_vecMtlID[i % m_vecMtlID.size()]));
			ENGINE_CALL_CHECK(SceneNodeRebase(m_nMainScene, nNode));
			ENGINE_CALL_CHECK(SetSceneNodePosition(nNode, util::RandomGenerator::RandomSampleSphere(20.f)));
			ENGINE_CALL_CHECK(SetSceneNodeEulerRotation(nNode, 
				{util::RandomGenerator::UniformDistribution<float>(0, glm::pi<float>() * 2.f),
				util::RandomGenerator::UniformDistribution<float>(0, glm::pi<float>() * 2.f), 
				util::RandomGenerator::UniformDistribution<float>(0, glm::pi<float>() * 2.f)}));
		}
		pDisplayDivice->SetView(m_pView);
	}

	void Engine::UpdateDebugScene()
	{
		static util::Timer timer;

		const double dCurTime = timer.GetTick();


		m_nFrameIndex++;

		const float tValue = dCurTime / 4000.0 * glm::pi<float>() * 2.f;

		const float fRadius = 50.f;
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

		for (ImageID nID : m_vecMtlID) {
			DeleteMaterial(nID);
		}

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


	EngineResult Engine::CreateLightNode(NodeID& nOutNodeID, NodeID nParentID, LightType eLightType)
	{
		NodeType eNodeType = EngineHelper::LightTypeToNodeType(eLightType);
		if (eNodeType == NodeType::eUnknown)
		{
			nOutNodeID = INVALID_NODE_ID;
			return EngineResult::eInvalidParam;
		}

		nOutNodeID = SceneManager::CreateNode(eNodeType);
		Node* pLightNode = SceneManager::GetNode(nOutNodeID);
		if (!pLightNode)
		{
			nOutNodeID = INVALID_NODE_ID;
			return EngineResult::eFailed;
		}

		// 标记为灯光节点
		pLightNode->SetNodeFlag(static_cast<uint32_t>(pLightNode->GetNodeFlag()) | static_cast<uint32_t>(NodeFlagBits::eIsLightNode));

		// 挂到父节点下
		if (nParentID != INVALID_NODE_ID)
		{
			EngineResult nResult = SceneNodeRebase(nParentID, nOutNodeID);
			if (nResult != EngineResult::eSuccess)
			{
				SceneManager::ReleaseNode(nOutNodeID);
				nOutNodeID = INVALID_NODE_ID;
				return nResult;
			}
		}

		// 标记灯光数据有更新 使View在下一帧重新收集灯光
		EngineHelper::MarkLightDirty(nOutNodeID);

		return EngineResult::eSuccess;
	}

	EngineResult Engine::SetLightColor(NodeID nNodeID, const std::array<float, 3>& arrColor)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (!pNode)
			return EngineResult::eNoSceneNode;

		if (NodeLight* pLightNode = EngineHelper::GetNodeLight(pNode))
		{
			pLightNode->SetLightColor(arrColor[0], arrColor[1], arrColor[2]);
			EngineHelper::MarkLightDirty(nNodeID);
			return EngineResult::eSuccess;
		}

		return EngineResult::eNoComponent;
	}

	EngineResult Engine::SetLightRange(NodeID nNodeID, float fRange)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (!pNode)
			return EngineResult::eNoSceneNode;

		if (auto* pPointLight = EngineHelper::GetLightNodeChecked<NodePointLight>(pNode, LightType::eLightPoint))
		{
			pPointLight->SetRange(fRange);
		}
		else if (auto* pSpotLight = EngineHelper::GetLightNodeChecked<NodeSpotLight>(pNode, LightType::eLightSpot))
		{
			pSpotLight->SetRange(fRange);
		}
		else
			return EngineResult::eNoComponent;

		EngineHelper::MarkLightDirty(nNodeID);
		return EngineResult::eSuccess;
	}

	EngineResult Engine::SetLightRadius(NodeID nNodeID, float fRadius)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (!pNode)
			return EngineResult::eNoSceneNode;

		if (auto* pSphereLight = EngineHelper::GetLightNodeChecked<NodeSphereLight>(pNode, LightType::eLightSphere))
		{
			pSphereLight->SetRadius(fRadius);
			EngineHelper::MarkLightDirty(nNodeID);
			return EngineResult::eSuccess;
		}

		return EngineResult::eNoComponent;
	}

	EngineResult Engine::SetLightRectSize(NodeID nNodeID, const std::array<float, 2>& arrRectSize)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (!pNode)
			return EngineResult::eNoSceneNode;

		if (auto* pRectLight = EngineHelper::GetLightNodeChecked<NodeRectLight>(pNode, LightType::eLightRect))
		{
			// [0]U方向半宽 [1]V方向半高
			pRectLight->SetRectU(arrRectSize[0]);
			pRectLight->SetRectV(arrRectSize[1]);
			EngineHelper::MarkLightDirty(nNodeID);
			return EngineResult::eSuccess;
		}

		return EngineResult::eNoComponent;
	}

	EngineResult Engine::SetLightSpotRadian(NodeID nNodeID, const std::array<float, 2>& arrInnerOuterRadian)
	{
		Node* pNode = SceneManager::GetNode(nNodeID);
		if (!pNode)
			return EngineResult::eNoSceneNode;

		if (auto* pSpotLight = EngineHelper::GetLightNodeChecked<NodeSpotLight>(pNode, LightType::eLightSpot))
		{
			// [0]内锥角 [1]外锥角
			float fInner = arrInnerOuterRadian[0];
			float fOuter = arrInnerOuterRadian[1];
			// 先临时放宽外锥角 保证设置顺序不会触发断言(内锥角必须不大于外锥角)
			pSpotLight->SetOuterRadian(std::max(fOuter, pSpotLight->GetOuterRadian()));
			pSpotLight->SetInnerRadian(fInner);
			pSpotLight->SetOuterRadian(fOuter);
			EngineHelper::MarkLightDirty(nNodeID);
			return EngineResult::eSuccess;
		}

		return EngineResult::eNoComponent;
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
		pOutDisplaySurface = new DisplayWindowSurface(surface, nWidth, nHeight);

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
		EngineResult result = EngineResult::eSuccess;
		MaterialRef refMtl = MaterialManager::CreateMaterial(eMtlType);
		if (!refMtl)
		{
			refMtl = MaterialManager::GetInstance().GetDefaultErrorMaterial();
			result = EngineResult::eFailed;
		}
		nOutMtlID = refMtl.GetID();
		MaterialManager::GetInstance().RefIncrease(nOutMtlID);
		return result;
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
