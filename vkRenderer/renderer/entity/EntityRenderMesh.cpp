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

		//for (auto& nBufferHandle : m_arrConstBufferVertTrans)
		//{
		//	nBufferHandle = ShaderResourceManager::CreateTransBufferHandle();
		//}

	}
	EntityRenderMesh::~EntityRenderMesh()
	{
		//for (ConstBufferHandle& hBuffer : m_arrConstBufferVertTrans)
		//{
		//	ShaderResourceManager::ReleaseTransBufferHandle(hBuffer);
		//	hBuffer = INVALID_CONST_BUFFER_HANDLE;

		//}
	}

	void EntityRenderMesh::SetMaterial(const MaterialRef& refMtl) {
		m_refMaterial = refMtl;
	}

	MaterialRef EntityRenderMesh::GetMaterial() const
	{
		return m_refMaterial;
	}

	MaterialID EntityRenderMesh::GetMaterialID() const
	{
		return m_refMaterial.GetID();
	}

	void EntityRenderMesh::SetVertexBuffer(VertexBuffer* pVertex) {
		m_pVertexBuffer = pVertex;
	}
	void EntityRenderMesh::SetIndexBuffer(IndexBuffer* pIndex) {
		m_pIndexBuffer = pIndex;
	}

	void EntityRenderMesh::UpdateTransBuffer(const EntityDrawInfo& sDrawInfo) {
		//sDrawInfo.pRenderView->SetModelMat(m_matModel);
		//if (RenderViewSingleCamera* pRenderView = dynamic_cast<RenderViewSingleCamera*>(sDrawInfo.pRenderView))
		//{
		//	ShaderResourceManager::UpdateTransBuffer(m_arrConstBufferVertTrans[sDrawInfo.nFlightFrameIndex], pRenderView->GetTransBuffer());
		//}
		//else
		//{
		//	RENDERER_ASSERT(false, "Unsupport Render View.");
		//}
	}

	void EntityRender::SetModuleMat(const glm::mat4& matMod)
	{
		m_matModel = matMod;

		m_sModelNormalMat.modelMat = m_matModel;
		glm::mat4 matNormal = glm::transpose(glm::inverse(m_matModel));

		memcpy(&m_sModelNormalMat.normalMatCol, glm::value_ptr(matNormal), sizeof(m_sModelNormalMat.normalMatCol));
	}

	void EntityRenderMesh::Draw(EntityDrawInfo& sInOutDrawInfo)
	{
		CommandBufferWrapper* pCmdBuffer = sInOutDrawInfo.pCommandBuffer;

		pCmdBuffer->BindIndexBuffer(m_pIndexBuffer->GetNativeBuffer(), 0, vk::IndexType::eUint32);
		std::vector<vk::Buffer> vertexBuffers{ m_pVertexBuffer->GetNativeBuffer() };
		std::vector<vk::DeviceSize> vertexOffsets{ 0 };
		pCmdBuffer->BindVertexBuffers(0, vertexBuffers, vertexOffsets);
		
		pCmdBuffer->PushConstants(sInOutDrawInfo.vkLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ModelNormalMat), &m_sModelNormalMat);

		pCmdBuffer->DrawIndexed(m_pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);

		//// 设置变换矩阵
		//sInOutDrawInfo.pRenderView->SetModelMat(m_matModel);
		//// TODO: 处理变换矩阵的更新
		//UpdateTransBuffer(sInOutDrawInfo);
		//ShaderResourceManager::UpdateDeviceTransBuffer();

		//// TransBuffer绑定到ShaderResource
		//m_refMaterial->SetTransBuffer(m_arrConstBufferVertTrans[sInOutDrawInfo.nFlightFrameIndex]);

		//MaterialBindInfo sBindInfo;
		//sBindInfo.eStage = sInOutDrawInfo.eRenderStage;
		//sBindInfo.nFlag = m_eRenderPassFlag;
		//sBindInfo.nFlightIndex = sInOutDrawInfo.nFlightFrameIndex;

		//GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(m_refMaterial->Bind(sBindInfo));
		//if (pRenderPass)
		//{
		//	RecordCommandInfo sRecordInfo;
		//	sRecordInfo.nWidth = sInOutDrawInfo.nWidth;
		//	sRecordInfo.nHeight = sInOutDrawInfo.nHeight;
		//	sRecordInfo.nDepthStencilID = sInOutDrawInfo.nDepthBuffer;
		//	sRecordInfo.vecImageIDColor = sInOutDrawInfo.vecRenderTargets;
		//	sRecordInfo.vecVertexBufferID.push_back(m_pVertexBuffer->GetBufferID());
		//	sRecordInfo.nIndexBufferID = m_pIndexBuffer->GetBufferID();
		//	sRecordInfo.nFlightFrameIndex = sInOutDrawInfo.nFlightFrameIndex;
		//	sRecordInfo.eColorLoadOp = sInOutDrawInfo.bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
		//	sRecordInfo.eDepthLoadOp = sInOutDrawInfo.bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;

		//	pRenderPass->RecordCommand(sRecordInfo);

		//	GraphicSubmitInfo sSubmitInfo;
		//	sSubmitInfo.nFlightFrameIndex = sInOutDrawInfo.nFlightFrameIndex;

		//	if (!sInOutDrawInfo.vecSemWait.empty())
		//	{
		//		sSubmitInfo.vecSemToWait = sInOutDrawInfo.vecSemWait;
		//		sSubmitInfo.vecSemWaitMasks = sInOutDrawInfo.vecSemWaitMasks;
		//	}

		//	if (!sInOutDrawInfo.vecSemSignal.empty())
		//	{
		//		sSubmitInfo.vecSemToSignal = sInOutDrawInfo.vecSemSignal;
		//	}

		//	pRenderPass->Submit(sSubmitInfo);

		//	sInOutDrawInfo.refFence = sSubmitInfo.refFence;
		//}
	}
}