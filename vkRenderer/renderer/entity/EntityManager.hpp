// Entity管理器
#pragma once
#include "IEntity.hpp"
#include "ManagerTemplate.hpp"

namespace LT {
	class EntityRenderMesh;
	DECLEAR_SINGLETON_MANAGER_BEGIN(EntityManager, IEntity, EntityID, Entity)

public:
	static _EntityRef CreateEntityRenderMesh();

	DECLEAR_SINGLETON_MANAGER_END(EntityManager, IEntity, EntityID, Entity)

} // namespace
