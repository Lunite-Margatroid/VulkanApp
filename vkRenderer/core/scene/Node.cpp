// 场景树节点
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "Node.hpp"

namespace LT {
	Node::Node(NodeID id)
		: m_id(id), m_pParent(nullptr)
	{
		m_arrComponents.fill(nullptr);
	}

	Node::~Node() {
		// 父节点拥有子节点的所有权 析构时级联释放所有子节点
		for (Node* pChild : m_listChildren)
		{
			delete pChild;
		}
	}

	Node* Node::GetParent() const {
		return m_pParent;
	}

	const std::list<Node*>& Node::GetChildren() const {
		return m_listChildren;
	}

	void Node::AddChild(Node* pChild) {
		RENDERER_ASSERT(pChild, "Node::AddChild: pChild is nullptr.");
		RENDERER_ASSERT(pChild->m_pParent == nullptr, "Node::AddChild: pChild already has a parent.");
		// 防止构成环
		for (const Node* p = this; p; p = p->m_pParent)
		{
			RENDERER_ASSERT(p != pChild, "Node::AddChild: pChild is an ancestor of this node.");
		}

		pChild->m_pParent = this;
		m_listChildren.push_back(pChild);
	}

	bool Node::RemoveChild(Node* pChild) {
		auto iter = std::find(m_listChildren.begin(), m_listChildren.end(), pChild);
		if (iter == m_listChildren.end())
		{
			return false;
		}
		m_listChildren.erase(iter);
		pChild->m_pParent = nullptr;
		return true;
	}
} // namespace LT
