// 渲染实体Mesh
#pragma once
#include "EntityRender.hpp"
#include "IMesh.hpp"
#include "MeshManager.hpp"
#include "MaterialManager.hpp"
#include "IMaterial.hpp"

namespace LT {
	class EntityRenderMesh : public EntityRender{
	protected:
		MeshRef m_refMesh;
		MaterialID m_nMaterialID;

	public:
		EntityRenderMesh(EntityID nID);
		~EntityRenderMesh() = default;

		EntityRenderMesh(EntityRenderMesh&&) = delete;
		EntityRenderMesh(const EntityRenderMesh&) = delete;
		EntityRenderMesh& operator = (EntityRenderMesh&&) = delete;
		EntityRenderMesh& operator = (const EntityRenderMesh&) = delete;

	};

} // namespace