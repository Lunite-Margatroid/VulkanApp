// 渲染实体Mesh
#include "vkRendererCommon.h"
#include "EntityRenderMesh.hpp"
#include "GraphicPass.hpp"

namespace LT {
	EntityRenderMesh::EntityRenderMesh(EntityID nID)
		:EntityRender(nID)
		,m_refMesh(INVALID_MESH_ID)
		,m_refMaterial(INVALID_MATERIAL_ID)
		,m_pVertexBuffer(nullptr)
		,m_pIndexBuffer(nullptr)
		,m_eRenderPassFlag(0)
	{
		SetBackCull(m_eRenderPassFlag, false);
		SetClockwiseFront(m_eRenderPassFlag, false);
		SetLineWidth(m_eRenderPassFlag, 1.f);
		SetBlendEnable(m_eRenderPassFlag, true);
		SetPolygonMode(m_eRenderPassFlag, vk::PolygonMode::eFill);
	}
	void EntityRenderMesh::SetMesh(const MeshRef& refMesh)
	{
		m_refMesh = refMesh;
		auto nFlag = m_refMesh->GetRenderPassFlag();
		SetPrimitiveTopology(m_eRenderPassFlag, GetPrimitiveTopology(nFlag));
		m_eRenderPassFlag = (VERTEX_CHANNEL_FLAG_MASK & nFlag) | (m_eRenderPassFlag & ~VERTEX_CHANNEL_FLAG_MASK);
	}

	void EntityRenderMesh::SetMaterial(const MaterialRef& refMtl) {
		m_refMaterial = refMtl;
	}

	void EntityRenderMesh::SetVertexBuffer(VertexBuffer* pVertex) {
		m_pVertexBuffer = pVertex;
	}
	void EntityRenderMesh::SetIndexBuffer(IndexBuffer* pIndex) {
		m_pIndexBuffer = pIndex;
	}
	void EntityRenderMesh::Draw(const EntityDrawInfo& sDrawInfo)
	{
		GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(m_refMaterial->GetRenderPass(sDrawInfo.m_eRenderStage, m_eRenderPassFlag));
		if (pRenderPass)
		{
			
		}
	}
}