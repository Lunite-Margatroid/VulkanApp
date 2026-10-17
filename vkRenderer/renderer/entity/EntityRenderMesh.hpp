// 渲染实体Mesh
#pragma once
#include "EntityRender.hpp"
#include "MeshManager.hpp"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "RenderPass.hpp"

namespace LT {
	class EntityRenderMesh : public EntityRender {
	protected:
		MaterialRef m_refMaterial;
		VertexBuffer* m_pVertexBuffer;
		IndexBuffer* m_pIndexBuffer;

		RenderPassFlag m_eRenderPassFlag;

		//std::array<ConstBufferHandle, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrConstBufferVertTrans;

	public:
		EntityRenderMesh(EntityID nID);
		~EntityRenderMesh();

		EntityRenderMesh(EntityRenderMesh&&) = delete;
		EntityRenderMesh(const EntityRenderMesh&) = delete;
		EntityRenderMesh& operator = (EntityRenderMesh&&) = delete;
		EntityRenderMesh& operator = (const EntityRenderMesh&) = delete;

		void SetMaterial(const MaterialRef& refMtl) override;
		MaterialRef GetMaterial() const;
		MaterialID GetMaterialID() const;
		void SetVertexBuffer(VertexBuffer* pVertex);
		void SetIndexBuffer(IndexBuffer* pIndex);

		void UpdateTransBuffer(const EntityDrawInfo& sDrawInfo) override;
		void Draw(EntityDrawInfo& sInOutDrawInfo) override;

		void SetRenderPassFlag(RenderPassFlag nFlag) override { m_eRenderPassFlag = nFlag; }
		RenderPassFlag GetRenderPassFlag() const override { return m_eRenderPassFlag; }
	};

} // namespace