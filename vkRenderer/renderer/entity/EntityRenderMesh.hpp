// 渲染实体Mesh
#pragma once
#include "EntityRender.hpp"
#include "MeshManager.hpp"
#include "MaterialManager.hpp"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "RenderPass.hpp"

namespace LT {
	class EntityRenderMesh : public EntityRender{
	protected:
		MeshRef m_refMesh;
		MaterialRef m_refMaterial;
		VertexBuffer* m_pVertexBuffer;
		IndexBuffer* m_pIndexBuffer;

		RenderPassFlag m_eRenderPassFlag;

	public:
		EntityRenderMesh(EntityID nID);
		~EntityRenderMesh() = default;

		EntityRenderMesh(EntityRenderMesh&&) = delete;
		EntityRenderMesh(const EntityRenderMesh&) = delete;
		EntityRenderMesh& operator = (EntityRenderMesh&&) = delete;
		EntityRenderMesh& operator = (const EntityRenderMesh&) = delete;

		void SetMesh(const MeshRef& refMesh);
		void SetMaterial(const MaterialRef& refMtl);
		void SetVertexBuffer(VertexBuffer* pVertex);
		void SetIndexBuffer(IndexBuffer* pIndex);

		void Draw(const EntityDrawInfo& sDrawInfo) override;
	};

} // namespace