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
	static _MeshRef CreateCube(float fSideLength);
	// 经细分最小为3 纬细分最小为1(此时退化为三角六面体)
	static _MeshRef CreateSphere(float fRadius, uint32_t nLongSubdivision, uint32_t nLatSubdivision);

	//DECLEAR_SINGLETON_MANAGER_END(MeshManager, IMesh, MeshID, Mesh)
	DECLEAR_SINGLETON_MANAGER_END(MeshManager, IMesh, MeshID, Mesh)


} // namespace