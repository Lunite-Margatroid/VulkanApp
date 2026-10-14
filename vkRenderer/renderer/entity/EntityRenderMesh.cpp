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
		for (ConstBufferHandle& hBuffer : m_arrConstBufferVertTrans)
		{
			ShaderResourceManager::ReleaseTransBufferHandle(hBuffer);
			hBuffer = INVALID_CONST_BUFFER_HANDLE;

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
		sDrawInfo.pRenderView->SetModelMat(m_matModule);
		if (RenderViewSingleCamera* pRenderView = dynamic_cast<RenderViewSingleCamera*>(sDrawInfo.pRenderView))
		{
			ShaderResourceManager::UpdateTransBuffer(m_arrConstBufferVertTrans[sDrawInfo.nFlightFrameIndex], pRenderView->GetTransBuffer());
		}
		else
		{
			RENDERER_ASSERT(false, "Unsupport Render View.");
		}
	}

	void EntityRender::SetModuleMat(const glm::mat4& matMod)
	{
		m_matModule = matMod;
	}

	void EntityRenderMesh::Draw(EntityDrawInfo& sInOutDrawInfo)
	{
		// 设置变换矩阵
		sInOutDrawInfo.pRenderView->SetModelMat(m_matModule);
		// TODO: 处理变换矩阵的更新
		UpdateTransBuffer(sInOutDrawInfo);
		ShaderResourceManager::UpdateDeviceTransBuffer();

		// TransBuffer绑定到ShaderResource
		m_refMaterial->SetTransBuffer(m_arrConstBufferVertTrans[sInOutDrawInfo.nFlightFrameIndex]);

		MaterialBindInfo sBindInfo;
		sBindInfo.eStage = sInOutDrawInfo.eRenderStage;
		sBindInfo.nFlag = m_eRenderPassFlag;
		sBindInfo.nFlightIndex = sInOutDrawInfo.nFlightFrameIndex;

		GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(m_refMaterial->Bind(sBindInfo));
		if (pRenderPass)
		{
			RecordCommandInfo sRecordInfo;
			sRecordInfo.nWidth = sInOutDrawInfo.nWidth;
			sRecordInfo.nHeight = sInOutDrawInfo.nHeight;
			sRecordInfo.nDepthStencilID = sInOutDrawInfo.nDepthBuffer;
			sRecordInfo.vecImageIDColor = sInOutDrawInfo.vecRenderTargets;
			sRecordInfo.vecVertexBufferID.push_back(m_pVertexBuffer->GetBufferID());
			sRecordInfo.nIndexBufferID = m_pIndexBuffer->GetBufferID();
			sRecordInfo.nFlightFrameIndex = sInOutDrawInfo.nFlightFrameIndex;

			pRenderPass->RecordCommand(sRecordInfo);

			GraphicSubmitInfo sSubmitInfo;
			sSubmitInfo.nFlightFrameIndex = sInOutDrawInfo.nFlightFrameIndex;

			if (!sInOutDrawInfo.vecSemWait.empty())
			{
				sSubmitInfo.vecSemToWait = sInOutDrawInfo.vecSemWait;
				sSubmitInfo.vecSemWaitMasks = sInOutDrawInfo.vecSemWaitMasks;
			}

			if (!sInOutDrawInfo.vecSemSignal.empty())
			{
				sSubmitInfo.vecSemToSignal = sInOutDrawInfo.vecSemSignal;
			}

			pRenderPass->Submit(sSubmitInfo);

			sInOutDrawInfo.refFence = sSubmitInfo.refFence;
		}
	}
}