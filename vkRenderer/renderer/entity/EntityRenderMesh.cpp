// 渲染实体Mesh
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "EntityRenderMesh.hpp"
#include "GraphicPass.hpp"
#include "BufferManager.h"
#include "RenderViewSingleCamera.hpp"

namespace LT {
	EntityRenderMesh::EntityRenderMesh(EntityID nID)
		:EntityRender(nID)
		,m_refMesh(INVALID_MESH_ID)
		,m_refMaterial(INVALID_MATERIAL_ID)
		,m_pVertexBuffer(nullptr)
		,m_pIndexBuffer(nullptr)
		,m_eRenderPassFlag(0)
		,m_matModel(glm::identity<glm::mat4>())
	{
		SetBackCull(m_eRenderPassFlag, false);
		SetClockwiseFront(m_eRenderPassFlag, false);
		SetLineWidth(m_eRenderPassFlag, 1.f);
		SetBlendEnable(m_eRenderPassFlag, true);
		SetPolygonMode(m_eRenderPassFlag, vk::PolygonMode::eFill);

		MVPMatrixBuffer tMVPBuf;

		for (auto& idBuffer : m_arrConstBufferVertTrans)
		{
			idBuffer = BufferManager::CreateConstBuffer(sizeof(tMVPBuf), &tMVPBuf)->GetBufferID();
		}

	}
	EntityRenderMesh::~EntityRenderMesh()
	{
		for (auto& idBuffer : m_arrConstBufferVertTrans)
		{
			BufferManager::DeleteBuffer(idBuffer);
			idBuffer = INVALID_BUFFER_ID;
		}
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
		GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(m_refMaterial->GetRenderPass(sDrawInfo.eRenderStage, m_eRenderPassFlag));
		m_refMaterial->UpdateMtlResource(sDrawInfo.eRenderStage, m_eRenderPassFlag, sDrawInfo.nFlightFrameIndex);
		if (pRenderPass)
		{
			ConstBuffer* pConstBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(m_arrConstBufferVertTrans[sDrawInfo.nFlightFrameIndex]));
			if (pConstBuffer)
			{
				RenderViewSingleCamera* pRenderView = dynamic_cast<RenderViewSingleCamera*>(sDrawInfo.pRenderView);
				pRenderView->SetModelMat(m_matModel);
				pConstBuffer->UpdateConstBuffer(pRenderView->GetTransBuffer());
			}

			pRenderPass->BindConstBuffer(m_arrConstBufferVertTrans[sDrawInfo.nFlightFrameIndex], BindingSpace::eVertexShader, 0, sDrawInfo.nFlightFrameIndex);

			RecordCommandInfo sRecordInfo;
			sRecordInfo.nWidth = sDrawInfo.nWidth;
			sRecordInfo.nHeight = sDrawInfo.nHeight;
			sRecordInfo.nDepthStencilID = sDrawInfo.nDepthBuffer;
			sRecordInfo.vecImageIDColor = sDrawInfo.vecRenderTargets;
			sRecordInfo.vecVertexBufferID.push_back(m_pVertexBuffer->GetBufferID());
			sRecordInfo.nIndexBufferID = m_pIndexBuffer->GetBufferID();
			sRecordInfo.nFlightFrameIndex = sDrawInfo.nFlightFrameIndex;

			pRenderPass->RecordCommand(sRecordInfo);

			GraphicSubmitInfo sSubmitInfo;
			sSubmitInfo.nFlightFrameIndex = sDrawInfo.nFlightFrameIndex;

			if (!sDrawInfo.vecSemWait.empty())
			{
				sSubmitInfo.vecSemToWait = sDrawInfo.vecSemWait;
				sSubmitInfo.vecSemWaitMasks = sDrawInfo.vecSemWaitMasks;
			}

			if (!sDrawInfo.vecSemSignal.empty())
			{
				sSubmitInfo.vecSemToSignal = sDrawInfo.vecSemSignal;
			}

			if (sDrawInfo.vkFenceSet)
			{
				sSubmitInfo.vkFenceToSet = sDrawInfo.vkFenceSet;
			}

			pRenderPass->Submit(sSubmitInfo);
		}
	}
}