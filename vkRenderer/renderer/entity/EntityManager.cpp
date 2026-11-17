// Entity管理器
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "EntityManager.hpp"
#include "EntityRenderMesh.hpp"
#include "EntityRenderLogLine.hpp"

namespace LT {
	IMPLEMENT_SINGLETON_MANAGER(EntityManager, IEntity, EntityID, Entity)

	EntityRef EntityManager::CreateEntityRenderMesh() {
		EntityManager& mgr = GetInstance();
		EntityID id = mgr.GenID();
		EntityRenderMesh* pEntity = new EntityRenderMesh(id);
		return mgr.Insert(id, reinterpret_cast<IEntity*>(pEntity));
	}
	EntityRef EntityManager::CreateEntityRenderLogLine()
	{
		EntityManager& mgr = GetInstance();
		EntityID id = mgr.GenID();
		EntityRenderLogLine* pEntity = new EntityRenderLogLine(id);
		return mgr.Insert(id, reinterpret_cast<IEntity*>(pEntity));
	}
} // namespace LT
