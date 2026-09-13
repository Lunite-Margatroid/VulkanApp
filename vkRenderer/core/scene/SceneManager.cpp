// 场景管理器
#include "vkRendererCommon.h"
#include "SceneManager.hpp"

namespace LT {
	SceneManager* SceneManager::s_pSceneManager = nullptr;

	SceneManager::SceneManager() 
		:m_nIDCounter(0)
	{
	}

	NodeID SceneManager::GenID() {
		return static_cast<NodeID>(m_nIDCounter++);
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
