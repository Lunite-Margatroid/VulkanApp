// 场景管理器
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "SceneManager.hpp"
#include "NodeMesh.hpp"
#include "NodePointLight.hpp"
#include "NodeSphereLight.hpp"
#include "NodeSpotLight.hpp"
#include "NodeRectLight.hpp"
#include "NodeDirectionalLight.hpp"
#include "NodeAmbient.hpp"

namespace LT {
	SceneManager* SceneManager::s_pSceneManager = nullptr;

	SceneManager::SceneManager() 
		:m_nIDCounter(0)
	{
	}

	SceneManager::~SceneManager() {
	}

	void SceneManager::Init() {
		if (!s_pSceneManager)
		{
			s_pSceneManager = new SceneManager();
		}
	}

	void SceneManager::Release() {
		if (s_pSceneManager)
		{
			delete s_pSceneManager;
		}
		s_pSceneManager = nullptr;
	}

	SceneManager& SceneManager::GetInstance() {
		RENDERER_ASSERT(s_pSceneManager, "SceneManager Did not init.");
		return *s_pSceneManager;
	}

	Node* SceneManager::CreateNode() {
		SceneManager& instance = GetInstance();
		NodeID id = instance.GenID();
		Node* pNode = new Node(id);
		instance.m_mapNodes[id] = pNode;
		return pNode;
	}

	NodeID SceneManager::CreateNode(NodeType eNodeType)
	{
		SceneManager& instance = GetInstance();

		NodeID id = INVALID_NODE_ID;
		Node* pNode = nullptr;

		switch (eNodeType) {
			case NodeType::eNode:
				id = instance.GenID();
				pNode = new Node(id);
				break;
			case NodeType::eNodeMesh:
				id = instance.GenID();
				pNode = new NodeMesh(id);
				break;
			case NodeType::eNodePointLight:
				id = instance.GenID();
				pNode = new NodePointLight(id);
				break;
			case NodeType::eNodeSphereLight:
				id = instance.GenID();
				pNode = new NodeSphereLight(id);
				break;
			case NodeType::eNodeSpotLight:
				id = instance.GenID();
				pNode = new NodeSpotLight(id);
				break;
			case NodeType::eNodeRectLight:
				id = instance.GenID();
				pNode = new NodeRectLight(id);
				break;
			case NodeType::eNodeDirectionalLight:
				id = instance.GenID();
				pNode = new NodeDirectionalLight(id);
				break;
			case NodeType::eNodeAmbient:
				id = instance.GenID();
				pNode = new NodeAmbient(id);
				break;
			default:
				break;
		}

		if (id > INVALID_NODE_ID)
		{
			instance.m_mapNodes[id] = pNode;
		}

		return id;
	}

	Node* SceneManager::GetNode(NodeID id)
	{
		SceneManager& instance = GetInstance();
		auto it = instance.m_mapNodes.find(id);
		if (it != instance.m_mapNodes.end()) {
			return it->second;
		}
		return nullptr;
	}

	void SceneManager::ReleaseNode(Node* pNode) {
		if (!pNode)
		{
			return;
		}
		ReleaseNode(pNode->GetID());
	}
	void SceneManager::ReleaseNode(NodeID id) {
		SceneManager& instance = GetInstance();
		auto it = instance.m_mapNodes.find(id);
		if (it == instance.m_mapNodes.end()) {
			return;
		}

		Node* pNode = it->second;

		// 1. 先从父节点摘下 避免父节点析构时再次delete该节点
		if (Node* pParent = pNode->GetParent()) {
			pParent->RemoveChild(pNode);
		}

		// 2. 递归释放子节点
		// 每轮先把子节点从pNode的子节点列表中摘下 再递归释放它
		// 不能直接依赖Node析构函数的级联delete 否则子节点的ID会残留在m_mapNodes中成为悬空项
		while (!pNode->GetChildren().empty()) {
			Node* pChild = pNode->GetChildren().front();
			pNode->RemoveChild(pChild);
			ReleaseNode(pChild->GetID());
		}

		// 3. 此时pNode已经没有子节点 直接删除并从映射中移除
		// 先取得迭代器再递归释放 递归只擦除其他元素 std::map的迭代器仍然有效
		delete pNode;
		instance.m_mapNodes.erase(it);
	}

} // namespace LT
