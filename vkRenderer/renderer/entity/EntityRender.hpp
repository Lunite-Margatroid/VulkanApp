// 渲染器实体
#pragma once
#include "IEntity.hpp"


namespace LT {
	class EntityRender :IEntity {
	protected:

	public:
		EntityRender(EntityID);
		~EntityRender() = default;

		EntityRender(EntityRender&&) = delete;
		EntityRender(const EntityRender&) = delete;
		EntityRender& operator = (EntityRender&&) = delete;
		EntityRender& operator = (const EntityRender&) = delete;
	};


} // namespace LT