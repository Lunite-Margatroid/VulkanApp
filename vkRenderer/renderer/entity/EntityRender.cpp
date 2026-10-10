// 渲染器实体
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "EntityRender.hpp"

namespace LT {
	EntityRender::EntityRender(EntityID nID)
		: IEntity(nID)
		, m_matModule(1.f)
	{
	}
} // namespace LT