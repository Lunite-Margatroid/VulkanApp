// Mesh管理器
#include "vkRendererCommon.h"
#include "MeshManager.hpp"
#include "MeshStatic.hpp"

namespace LT {
	IMPLEMENT_SINGLETON_MANAGER(MeshManager, IMesh, MeshID, Mesh)

	MeshRef MeshManager::CreateStaticMesh() {
		MeshManager& mgr = GetInstance();
		MeshID id = mgr.GenID();
		MeshStatic* pMesh = new MeshStatic(id);
		return mgr.Insert(id, pMesh);
	}
} // namespace LT