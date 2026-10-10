// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"

namespace LT {
	void RenderStageOpaqueForward::Execute(const StageExecuteInfo& sExecuteInfo)
	{
		if (sExecuteInfo.vecRenderEntity.size() <= 0)
			return;

		EntityDrawInfo sDrawInfo;
		sDrawInfo.nWidth = sExecuteInfo.nWidth;
		sDrawInfo.nHeight = sExecuteInfo.nHeight;
		sDrawInfo.nFlightFrameIndex = sExecuteInfo.nFlightFrameIndex;
		sDrawInfo.nDepthBuffer = sExecuteInfo.nDepthBufferID;
		sDrawInfo.vecRenderTargets = sExecuteInfo.vecRenderTarget;
		sDrawInfo.pRenderView = sExecuteInfo.pRenderView;
		sDrawInfo.eRenderStage = GetRenderStageType();

		if (!sExecuteInfo.vecSemWait.empty())
		{
			sDrawInfo.vecSemWait = sExecuteInfo.vecSemWait;
			sDrawInfo.vecSemWaitMasks = sExecuteInfo.vecSemWaitMask;
		}

		for (int i = 0; i < sExecuteInfo.vecRenderEntity.size() - 1; i++)
		{
			sExecuteInfo.vecRenderEntity[i]->Draw(sDrawInfo);
		}

		if (!sExecuteInfo.vecSemSignal.empty())
		{
			sDrawInfo.vecSemSignal = sExecuteInfo.vecSemSignal;
		}

		if (sExecuteInfo.fenceSet)
		{
			sDrawInfo.vkFenceSet = sExecuteInfo.fenceSet;
		}

		sExecuteInfo.vecRenderEntity.back()->Draw(sDrawInfo);
	}
} // namespace LT