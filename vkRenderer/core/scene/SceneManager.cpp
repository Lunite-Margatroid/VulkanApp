// 场景管理器
#include "vkRendererCommon.h"
#include "SceneManager.hpp"
#include "NodeMesh.hpp"

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
		ReleaseNode(pNode->GetID());
	}
	void SceneManager::ReleaseNode(NodeID id) {
		SceneManager& instance = GetInstance();
		auto it = instance.m_mapNodes.find(id);
		if (it != instance.m_mapNodes.end()) {
			delete it->second;
			instance.m_mapNodes.erase(it);
		}
	}

} // namespace LT
