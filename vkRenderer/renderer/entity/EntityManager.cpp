// Entity管理器
#include "vkRendererCommon.h"
#include "EntityManager.hpp"
#include "EntityRenderMesh.hpp"

namespace LT {
	IMPLEMENT_SINGLETON_MANAGER(EntityManager, IEntity, EntityID, Entity)

	EntityRef EntityManager::CreateEntityRenderMesh() {
		EntityManager& mgr = GetInstance();
		EntityID id = mgr.GenID();
		EntityRenderMesh* pEntity = new EntityRenderMesh(id);
		return mgr.Insert(id, reinterpret_cast<IEntity*>(pEntity));
	}
} // namespace LT
