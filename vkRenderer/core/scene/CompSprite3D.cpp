#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "CompSprite3D.hpp"
#include "MeshManager.hpp"
#include "BufferManager.h"

#include "EntityRenderMesh.hpp"

#include "BufferManager.h"

namespace LT {
	CompSprite3D::CompSprite3D()
	:m_matWorld(glm::identity<glm::mat4>()), m_refEntityRender(),
	m_pVertexBuffer(nullptr), m_pIndexBuffer(nullptr), m_nRenderPassFlag(0) {
		SetLineWidth(1.f);
		SetPolygonMode(vk::PolygonMode::eFill);
		SetBackCull(false);
		SetClockwiseFront(false);
	}
	CompSprite3D::~CompSprite3D() {
		if (m_pVertexBuffer) {
			BufferManager::DeleteBuffer(m_pVertexBuffer);
		}

		if (m_pIndexBuffer)
		{
			BufferManager::DeleteBuffer(m_pIndexBuffer);
		}

	}

	void CompSprite3D::SetMesh(MeshRef refMesh) {

		if (m_pVertexBuffer)
		{
			BufferManager::DeleteBuffer(m_pVertexBuffer);
			m_pVertexBuffer = nullptr;
		}
		if (m_pIndexBuffer)
		{
			BufferManager::DeleteBuffer(m_pIndexBuffer);
			m_pIndexBuffer = nullptr;
		}

		m_refEntityRender = EntityManager::CreateEntityRenderMesh();
		EntityRenderMesh* pEntity = dynamic_cast<EntityRenderMesh*>(m_refEntityRender.GetPtr());

		RenderPassFlag nMeshFlag = refMesh->GetRenderPassFlag();
		m_nRenderPassFlag = (m_nRenderPassFlag & (~VERTEX_CHANNEL_FLAG_MASK)) | nMeshFlag;
		SetPrimitiveTopology(LT::GetPrimitiveTopology(nMeshFlag));

		auto [pVertexBuffer, pIndexBuffer] = BufferManager::CreateVertexIndexBuffer(refMesh, m_nRenderPassFlag);

		m_pVertexBuffer = pVertexBuffer;
		m_pIndexBuffer = pIndexBuffer;

		pEntity->SetVertexBuffer(m_pVertexBuffer);
		pEntity->SetIndexBuffer(m_pIndexBuffer);
		pEntity->SetRenderPassFlag(m_nRenderPassFlag);

		if (m_refMtl)
		{
			pEntity->SetMaterial(m_refMtl);
		}
	}
	void CompSprite3D::SetMaterial(MaterialRef refMtl)
	{
		m_refMtl = refMtl;
		if (m_refEntityRender)
		{
			EntityRenderMesh* pEntity = dynamic_cast<EntityRenderMesh*>(m_refEntityRender.GetPtr());
			pEntity->SetMaterial(refMtl);
		}
	}
	EntityRef CompSprite3D::GetRenderEntity()
	{
		return m_refEntityRender;
	}
	void CompSprite3D::SetLineWidth(float fLineWidth)
	{
		LT::SetLineWidth(m_nRenderPassFlag, fLineWidth);
	}
	float CompSprite3D::GetLineWidth() const
	{
		return LT::GetLineWidth(m_nRenderPassFlag);
	}
	void CompSprite3D::SetPolygonMode(vk::PolygonMode ePolygonMode)
	{
		LT::SetPolygonMode(m_nRenderPassFlag, ePolygonMode);
	}
	vk::PolygonMode CompSprite3D::GetPolygonMode() const
	{
		return LT::GetPolygonMode(m_nRenderPassFlag);
	}
	void CompSprite3D::SetBackCull(bool bBackCull)
	{
		LT::SetBackCull(m_nRenderPassFlag, bBackCull);
	}
	bool CompSprite3D::GetBackCull() const
	{
		return LT::IsBackCull(m_nRenderPassFlag);
	}
	void CompSprite3D::SetClockwiseFront(bool bClockwiseFront)
	{
		LT::SetClockwiseFront(m_nRenderPassFlag, bClockwiseFront);
	}
	bool CompSprite3D::GetClockwiseFront() const
	{
		return LT::IsClockwiseFront(m_nRenderPassFlag);
	}
	void CompSprite3D::SetBlendEnabled(bool bBlend)
	{
		LT::SetBlendEnable(m_nRenderPassFlag, bBlend);
	}
	bool CompSprite3D::GetBlendEnabled() const
	{
		return LT::IsBlendEnabled(m_nRenderPassFlag);
	}
	void CompSprite3D::SetPrimitiveTopology(vk::PrimitiveTopology ePrimitiveTopology)
	{
		LT::SetPrimitiveTopology(m_nRenderPassFlag, ePrimitiveTopology);
	}
	vk::PrimitiveTopology CompSprite3D::GetPrimitiveTopology() const
	{
		return LT::GetPrimitiveTopology(m_nRenderPassFlag);
	}
	RenderPassFlag CompSprite3D::GetRenderPassFlag() const
	{
		return m_nRenderPassFlag;
	}
} // namespace LT