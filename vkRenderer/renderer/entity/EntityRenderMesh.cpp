// 渲染实体Mesh
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "EntityRenderMesh.hpp"
#include "GraphicPass.hpp"
#include "BufferManager.h"
#include "RenderViewSingleCamera.hpp"

#include "ShaderResourceManager.hpp"

namespace LT {
	EntityRenderMesh::EntityRenderMesh(EntityID nID)
		:EntityRender(nID)
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

		MVPMatrixBuffer tMVPBuf;

		for (auto& nBufferHandle : m_arrConstBufferVertTrans)
		{
			nBufferHandle = ShaderResourceManager::CreateTransBufferHandle();
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

	void EntityRenderMesh::SetMaterial(const MaterialRef& refMtl) {
		m_refMaterial = refMtl;
	}

	void EntityRenderMesh::SetVertexBuffer(VertexBuffer* pVertex) {
		m_pVertexBuffer = pVertex;
	}
	void EntityRenderMesh::SetIndexBuffer(IndexBuffer* pIndex) {
		m_pIndexBuffer = pIndex;
	}

	void EntityRenderMesh::UpdateTransBuffer(const EntityDrawInfo& sDrawInfo) {
		sDrawInfo.pRenderView->SetModelMat(sDrawInfo.matModule);
		if (RenderViewSingleCamera* pRenderView = dynamic_cast<RenderViewSingleCamera*>(sDrawInfo.pRenderView))
		{
			ShaderResourceManager::UpdateTransBuffer(m_arrConstBufferVertTrans[sDrawInfo.nFlightFrameIndex], pRenderView->GetTransBuffer());
		}
		else
		{
			RENDERER_ASSERT(false, "Unsupport Render View.");
		}
	}

	void EntityRenderMesh::Draw(const EntityDrawInfo& sDrawInfo)
	{
		GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(m_refMaterial->GetRenderPass(sDrawInfo.eRenderStage, m_eRenderPassFlag));
		if (pRenderPass)
		{
			// 设置变换矩阵
			sDrawInfo.pRenderView->SetModelMat(sDrawInfo.matModule);
			// TransBuffer绑定到ShaderResource
			m_refMaterial->SetTransBuffer(m_arrConstBufferVertTrans[sDrawInfo.nFlightFrameIndex]);
			// ShaderResource绑定到RenderPass
			m_refMaterial->UpdateMtlResource(sDrawInfo.eRenderStage, m_eRenderPassFlag);
			// 提交RenderPass的ShaderResource
			pRenderPass->BindResourceToDevice(sDrawInfo.nFlightFrameIndex);

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