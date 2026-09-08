// 渲染器实体
#pragma once
#include "IEntity.hpp"

#include "IRenderStage.hpp"

namespace LT {

	struct EntityDrawInfo {
		RenderStageType m_eRenderStage;
		std::vector<ImageID> m_vecRenderTargets;
	};

	class EntityRender :IEntity {
	protected:

	public:
		EntityRender(EntityID);
		~EntityRender() = default;

		EntityRender(EntityRender&&) = delete;
		EntityRender(const EntityRender&) = delete;
		EntityRender& operator = (EntityRender&&) = delete;
		EntityRender& operator = (const EntityRender&) = delete;

		virtual void Draw(const EntityDrawInfo& sDrawInfo) = 0;
	};


} // namespace LT