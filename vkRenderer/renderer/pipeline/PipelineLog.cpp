#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "PipelineLog.hpp"

#include "ShaderResourceManager.hpp"

#include "EntityRenderLogLine.hpp"

namespace LT {

	PipelineLog::PipelineLog(){
		for (int i = 0; i < m_arrTransBuffer.size(); ++i)
		{
			m_arrTransBuffer[i] = ShaderResourceManager::CreateCommonConstBuffer(sizeof(VPMatrixBuffer2D), nullptr);
		}
	}

	void PipelineLog::Resize(uint32_t nWidth, uint32_t nHeight) {
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}

	void PipelineLog::Execute(PipelineExecuteInfo& sInOutPipelineInfo) {
		FlightFrameIndex nFlightFrameIndex = sInOutPipelineInfo.nFightFrameIndex;
		ConstBufferHandle nTransBuffer = m_arrTransBuffer[nFlightFrameIndex];

		ShaderResourceManager::UpdateCommonConstBuffer(nTransBuffer,&m_sTransBuffer);

		StageExecuteInfo sStageExecuteInfo;
		sStageExecuteInfo.nWidth = m_nWidth;
		sStageExecuteInfo.nHeight = m_nHeight;
		sStageExecuteInfo.nFlightFrameIndex = nFlightFrameIndex;
		sStageExecuteInfo.nDepthBufferID = INVALID_IMAGE_ID;
		sStageExecuteInfo.vecRenderTarget = sInOutPipelineInfo.vecRenderTargets;
		sStageExecuteInfo.mapRenderEntity[MATERIAL_ID_2D_0] = m_vecLogLine;
		sStageExecuteInfo.pRenderView = nullptr;
		sStageExecuteInfo.vecSemWait = sInOutPipelineInfo.vecSemWait;
		sStageExecuteInfo.vecSemWaitMask = sInOutPipelineInfo.vecSemWaitMask;
		sStageExecuteInfo.vecSemSignal = sInOutPipelineInfo.vecSemSignal;

		sStageExecuteInfo.pPipeline = this;
		
		sStageExecuteInfo.vecColorClear  = sInOutPipelineInfo.vecColorClear;

		sStageExecuteInfo.bDepthClear = false;

		m_stageLog.Execute(sStageExecuteInfo);

		if (sStageExecuteInfo.refFence)
		{
			sInOutPipelineInfo.refFence = sStageExecuteInfo.refFence;
		}
	}

	void PipelineLog::SetColorInType(const std::vector<PipelineIOType>& vecColorInType)
	{
		Pipeline::SetColorInType(vecColorInType);
		m_stageLog.SetColorInType(vecColorInType);
	}

	void PipelineLog::SetColorOutType(const std::vector<PipelineIOType>&vecColorOutType)
	{
		Pipeline::SetColorOutType(vecColorOutType);
		m_stageLog.SetColorOutType(vecColorOutType);
	}

	void PipelineLog::SetDepthInType(PipelineIOType eDepthInType)
	{
		Pipeline::SetDepthInType(eDepthInType);
		m_stageLog.SetDepthInType(eDepthInType);
	}

	void PipelineLog::SetDepthOutType(PipelineIOType eDepthOutType)
	{
		Pipeline::SetDepthOutType(eDepthOutType);
		m_stageLog.SetDepthOutType(eDepthOutType);
	}

} // namespace LT
