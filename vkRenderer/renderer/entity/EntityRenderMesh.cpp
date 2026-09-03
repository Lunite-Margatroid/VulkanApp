// 渲染实体Mesh
#include "vkRendererCommon.h"
#include "EntityRenderMesh.hpp"

namespace LT {
	EntityRenderMesh::EntityRenderMesh(EntityID nID)
		:EntityRender(nID)
		,m_refMesh(INVALID_MESH_ID)
	{
	}
}