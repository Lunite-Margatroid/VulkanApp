// 组件 挂载到节点上

#pragma once

namespace LT {


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