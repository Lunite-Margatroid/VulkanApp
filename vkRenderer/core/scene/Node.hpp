// 场景树节点

#pragma once
#include <list>
#include <array>
#include "IComponent.hpp"
#include "CompSprite3D.hpp"
#include "CompTransform3D.hpp"
namespace LT {
	// 组件相关
	// Component
	// 映射
	// ---------------- Type->枚举 -------------------
	template<typename TComponent>
	ComponentType TypeOfComponent() {
		if constexpr (std::is_same<CompSprite3D, TComponent>{})
		{
			return ComponentType::eSprite3D;
		}
		else if constexpr (std::is_same<CompTransform3D, TComponent>{})
		{
			return ComponentType::eTransform3D;
		}
		else
		{
			RENDERER_ASSERT(false, "It is not Compoent Type");
			return ComponentType::eUnknown;
			//static_assert(false);
		}

	}

	// 映射
	// ---------------- 枚举->Type ---------------------
	template<ComponentType eType>
	struct ComponentTypeTraits;

	template<>
	struct ComponentTypeTraits<ComponentType::eTransform3D> { using type = CompTransform3D; };

	template<>
	struct ComponentTypeTraits<ComponentType::eSprite3D> { using type = CompSprite3D; };



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

		template<ComponentType eType>
		void AddComponent() {
			static_assert(eType < ComponentType::ComponentTypeCount && static_cast<int>(eType) >= 0);
			if (m_arrComponents[static_cast<int>(eType)] == nullptr)
			{
				m_arrComponents[static_cast<int>(eType)] = new ComponentTypeTraits<eType>::type();
			}
		}

		void AddComponent(ComponentType eType);

		void EraseComponent(ComponentType eType);

		// 遍历当前节点及其子节点
		// 广度优先
		void ForEachBFS_PreOrder(std::function<void(Node*)> func);

		template<typename TComponent>
		TComponent* GetComponent() {
			return reinterpret_cast<TComponent*>(m_arrComponents[static_cast<int>(TypeOfComponent<TComponent>())]);
		}


		IComponent* GetComponent(ComponentType eType);

		// 获取变换矩阵
		// 如果没有Transform Component 将从它的父节点获取
		glm::mat4 GetGlobalMat();
	};

} // namespace LT
