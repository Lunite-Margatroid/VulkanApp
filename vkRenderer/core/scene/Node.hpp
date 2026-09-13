// 场景树节点

#pragma once
#include <list>
#include <array>
#include "IComponent.hpp"

namespace LT {
	using NodeID = int64_t;

	constexpr NodeID INVALID_NODE_ID = INVALID_ITEM_ID;

	enum class NodeType : int {
		eUnknown = -1,
		eNode,
		eNodeMesh,
	};


	// 简单的树节点 用std::list<Node*>记录子节点
	// 父节点拥有子节点的所有权 析构时级联释放子节点
	class Node {
		friend class SceneManager;

	protected:
		const NodeID m_id;
		Node* m_pParent;
		std::list<Node*> m_listChildren;
		std::array<IComponent*, static_cast<size_t>(ComponentType::ComponentTypeCount)> m_arrComponents;

		Node(NodeID id);
		virtual ~Node();


	public:

		// 禁止拷贝和移动 节点具有唯一身份
		Node& operator = (const Node&) = delete;
		Node& operator = (Node&&) = delete;

		Node(const Node&) = delete;
		Node(Node&&) = delete;

		Node* GetParent() const;
		const std::list<Node*>& GetChildren() const;

		// 添加子节点 pChild必须是自由的(没有父节点) 且不能是自身或祖先节点
		void AddChild(Node* pChild);

		// 从子节点列表中移除 不删除pChild 所有权转交给调用者 移除后pChild的父节点为空
		// pChild不在子节点列表中时不产生任何效果 返回是否成功移除
		bool RemoveChild(Node* pChild);

		NodeID GetID() const { return m_id; }

		virtual NodeType GetNodeType() const { return NodeType::eNode; }
	};

} // namespace LT
