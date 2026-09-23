// 组件 挂载到节点上

#pragma once

namespace LT {
	// 这个枚举要作为索引
	enum class ComponentType : int{
		eUnknown = -1,
		eSprite3D,
		ComponentTypeCount,
		eCustomedComponent = 8192, // 自定义组件类型起始值
	};

	inline constexpr bool IsValidCompnentType(ComponentType eCompType) {
		return eCompType > ComponentType::eUnknown && eCompType < ComponentType::ComponentTypeCount;
	}

	// 组件 挂载到节点上
	class IComponent {
	public:
		virtual ~IComponent() = default;
		// 获取组件类型
		virtual ComponentType GetComponentType() const = 0;
	};
} // namespace LT