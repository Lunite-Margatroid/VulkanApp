// 场景管理器

#pragma once
#include "Node.hpp"

namespace LT {
	// 场景管理器 单例
	class SceneManager {
	private:
		int64_t m_nIDCounter;
		std::map<NodeID, Node*> m_mapNodes;

		SceneManager();

	public:
		~SceneManager();

		// 五法则 禁止拷贝和移动
		SceneManager& operator = (const SceneManager&) = delete;
		SceneManager& operator = (SceneManager&&) = delete;

		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;

	private:
		NodeID GenID() {
			return m_nIDCounter++;
		}

		// -------------- static ------------------
	private:
		static SceneManager* s_pSceneManager;
	public:
		static void Init();
		static void Release();
		static SceneManager& GetInstance();

		static Node* CreateNode();
		static NodeID CreateNode(NodeType eNodeType);

		static Node* GetNode(NodeID id);

		static void ReleaseNode(Node* pNode);
		static void ReleaseNode(NodeID id);
	};

} // namespace LT
