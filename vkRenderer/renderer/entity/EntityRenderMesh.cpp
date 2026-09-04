// 渲染实体Mesh
#include "vkRendererCommon.h"
#include "EntityRenderMesh.hpp"

namespace LT {
	EntityRenderMesh::EntityRenderMesh(EntityID nID)
		:EntityRender(nID)
		,m_refMesh(INVALID_MESH_ID)
		,m_refMaterial(INVALID_MATERIAL_ID)
	{
	}
	void EntityRenderMesh::SetMesh(const MeshRef& refMesh)
	{
		m_refMesh = refMesh;
	}

	void EntityRenderMesh::SetMaterial(const MaterialRef& refMtl) {
		m_refMaterial = refMtl;
	}
}