// Stage基类
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "IRenderStage.hpp"

#include "CommandBufferWrapper.hpp"

#include "SemaphoreManager.hpp"

namespace LT {
	IRenderStage::IRenderStage() 
	: m_eDepthInType(PipelineIOType::eUndefined)
	, m_eDepthOutType(PipelineIOType::eDepthAttachment)
	{
		// 创建Semaphore
		vk::Device& device = vkContext::GetVkDevice();
		for (int i = 0; i < m_arrSemTransitionBegin.size(); ++i)
		{
			m_arrSemTransitionBegin[i] = device.createSemaphore(vk::SemaphoreCreateInfo{});
		}

		for (int i = 0; i < m_arrSemTransitionEnd.size(); ++i)
		{
			m_arrSemTransitionEnd[i] = device.createSemaphore(vk::SemaphoreCreateInfo{});
		}
	}

	IRenderStage::~IRenderStage()
	{
		// 销毁Semaphore
		vk::Device& device = vkContext::GetVkDevice();
		for (int i = 0; i < m_arrSemTransitionBegin.size(); ++i)
		{
			if (m_arrSemTransitionBegin[i])
			{
				device.destroySemaphore(m_arrSemTransitionBegin[i]);
			}
		}

		for (int i = 0; i < m_arrSemTransitionEnd.size(); ++i)
		{
			if (m_arrSemTransitionEnd[i])
			{
				device.destroySemaphore(m_arrSemTransitionEnd[i]);
			}
		}
	}

	void IRenderStage::SetColorInType(const std::vector<PipelineIOType>& vecColorInType)
	{
		m_vecColorInType = vecColorInType;
	}

	void IRenderStage::SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType)
	{
		m_vecColorOutType = vecColorOutType;
	}

	void IRenderStage::SetDepthInType(PipelineIOType eDepthInType)
	{
		m_eDepthInType = eDepthInType;
	}

	void IRenderStage::SetDepthOutType(PipelineIOType eDepthOutType)
	{
		m_eDepthOutType = eDepthOutType;
	}


	void IRenderStage::StageBegin(StageBeginInfo& sInOutBeginInfo)
	{

		CommandBufferWrapper& cmdBuffer = vkContext::GetCmdBuffer(sInOutBeginInfo.nFlightFrameIndex);

		RecordTransitionInfo sRecordInfo;
		sRecordInfo.pCmdBuffer = &cmdBuffer;

		// Color Attacment
		uint32_t nColorAttachmentCount = std::min(sInOutBeginInfo.vecRenderTarget.size(), m_vecColorInType.size());
		for (int i = 0; i < nColorAttachmentCount; ++i)
		{
			PipelineIOType eInColorType = m_vecColorInType[i];

			if (eInColorType != PipelineIOType::eColorAttachment)
			{
				ImageID nImageID = sInOutBeginInfo.vecRenderTarget[i];

				sRecordInfo.vecColorAttachment.emplace_back(eInColorType, PipelineIOType::eColorAttachment, nImageID);
			}
		}


		// Depth Attachment
		if (sInOutBeginInfo.nDepthBufferID != INVALID_IMAGE_ID)
		{
			if (m_eDepthInType != PipelineIOType::eDepthAttachment)
			{
				sRecordInfo.vecDepthAttachment.emplace_back(m_eDepthInType, PipelineIOType::eDepthAttachment, sInOutBeginInfo.nDepthBufferID);
			}
		}

		// 如果有Image需要Transition
		if (sRecordInfo.vecColorAttachment.size() + sRecordInfo.vecDepthAttachment.size() > 0)
		{

			m_passTransition.RecordCommand(sRecordInfo);

			SemaphoreBarrier barrier = SemaphoreManager::Barrier(sInOutBeginInfo.nFlightFrameIndex, sInOutBeginInfo.refFence);

			
			TransitionSubmitInfo sSubmitInfo;
			sSubmitInfo.nFlightFrameIndex = sInOutBeginInfo.nFlightFrameIndex;
			//sSubmitInfo.vecSemToSignal = sInOutBeginInfo.vecSemSignal;
			//sSubmitInfo.vecSemToWait = sInOutBeginInfo.vecSemWait;
			//sSubmitInfo.vecSemWaitMasks = sInOutBeginInfo.vecSemWaitMask;
			sSubmitInfo.vecSemToSignal .push_back( barrier.m_vkSemaphoreToSignal);
			sSubmitInfo.vecSemToWait = barrier.GetWaitSemaphores();
			sSubmitInfo.vecSemWaitMasks.resize(sSubmitInfo.vecSemToWait.size(), vk::PipelineStageFlagBits::eAllGraphics);
			m_passTransition.Submit(sSubmitInfo);

			sInOutBeginInfo.refFence = sSubmitInfo.refFence;
		}
	}

	void IRenderStage::StageEnd(StageEndInfo& sInOutEndInfo) {
		RecordTransitionInfo sRecordInfo;
		sRecordInfo.pCmdBuffer = &vkContext::GetCmdBuffer(sInOutEndInfo.nFlightFrameIndex);
		// Color Attachment
		uint32_t nColorAttachmentCount = std::min(sInOutEndInfo.vecRenderTarget.size(), m_vecColorInType.size());
		for (int i = 0; i < nColorAttachmentCount; ++i) {
			PipelineIOType eOutColorType = m_vecColorOutType[i];

			if (eOutColorType != PipelineIOType::eColorAttachment && eOutColorType != PipelineIOType::eUndefined)
			{
				ImageID nImageID = sInOutEndInfo.vecRenderTarget[i];

				sRecordInfo.vecColorAttachment.emplace_back(PipelineIOType::eColorAttachment, eOutColorType, nImageID);
			}
		}

		// Depth Attachment
		if (m_eDepthOutType != PipelineIOType::eUndefined && m_eDepthOutType != PipelineIOType::eDepthAttachment) {
			sRecordInfo.vecColorAttachment.emplace_back(PipelineIOType::eDepthAttachment, m_eDepthOutType, sInOutEndInfo.nDepthBufferID);
		}

		if (sRecordInfo.vecColorAttachment.size() + sRecordInfo.vecDepthAttachment.size() > 0)
		{

			m_passTransition.RecordCommand(sRecordInfo);

			TransitionSubmitInfo sSubmitInfo;
			sSubmitInfo.nFlightFrameIndex = sInOutEndInfo.nFlightFrameIndex;
			sSubmitInfo.vecSemToSignal = sInOutEndInfo.vecSemSignal;
			sSubmitInfo.vecSemToWait = sInOutEndInfo.vecSemWait;
			sSubmitInfo.vecSemWaitMasks = sInOutEndInfo.vecSemWaitMask;

			m_passTransition.Submit(sSubmitInfo);

			sInOutEndInfo.refFence = sSubmitInfo.refFence;
		}
	}

	bool IRenderStage::OutNeedTransiton(std::span<ImageID> vecColorAttachments) const
	{
		// Color Attachment
		uint32_t nColorAttachmentCount = std::min(vecColorAttachments.size(), m_vecColorInType.size());
		for (int i = 0; i < nColorAttachmentCount; ++i) {
			PipelineIOType eOutColorType = m_vecColorOutType[i];

			if (eOutColorType != PipelineIOType::eColorAttachment && eOutColorType != PipelineIOType::eUndefined)
			{
				return true;
			}
		}

		// Depth Attachment
		if (m_eDepthOutType != PipelineIOType::eUndefined && m_eDepthOutType != PipelineIOType::eDepthAttachment) {
			return true;
		}

		return false;
	}

} // namespace 