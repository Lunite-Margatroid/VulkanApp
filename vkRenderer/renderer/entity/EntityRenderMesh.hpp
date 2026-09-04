// 渲染实体Mesh
#pragma once
#include "EntityRender.hpp"
#include "MeshManager.hpp"
#include "MaterialManager.hpp"

namespace LT {
	class EntityRenderMesh : public EntityRender{
	protected:
		MeshRef m_refMesh;
		MaterialRef m_refMaterial;

	public:
		EntityRenderMesh(EntityID nID);
		~EntityRenderMesh() = default;

		EntityRenderMesh(EntityRenderMesh&&) = delete;
		EntityRenderMesh(const EntityRenderMesh&) = delete;
		EntityRenderMesh& operator = (EntityRenderMesh&&) = delete;
		EntityRenderMesh& operator = (const EntityRenderMesh&) = delete;

		void SetMesh(const MeshRef& refMesh);
		void SetMaterial(const MaterialRef& refMtl);

	};

} // namespace