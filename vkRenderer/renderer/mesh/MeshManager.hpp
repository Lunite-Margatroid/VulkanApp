// Mesh管理器
#pragma once
#include "IMesh.hpp"
#include "ManagerTemplate.hpp"

namespace LT {
	class MeshStatic;
	//DECLEAR_SINGLETON_MANAGER_BEGIN(MeshManager, IMesh, MeshID, Mesh)
	DECLEAR_SINGLETON_MANAGER_BEGIN(MeshManager, IMesh, MeshID, Mesh)

public:
		static _MeshRef CreateStaticMesh();
	//DECLEAR_SINGLETON_MANAGER_END(MeshManager, IMesh, MeshID, Mesh)
	DECLEAR_SINGLETON_MANAGER_END(MeshManager, IMesh, MeshID, Mesh)


} // namespace