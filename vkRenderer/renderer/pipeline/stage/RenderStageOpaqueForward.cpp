// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"

namespace LT {
	void RenderStageOpaqueForward::Execute(const StageExecuteInfo& sExecuteInfo)
	{
		EntityDrawInfo sDrawInfo;
		sDrawInfo.nWidth = sExecuteInfo.nWidth;
		sDrawInfo.nHeight = sExecuteInfo.nHeight;
		sDrawInfo.nFlightFrameIndex = sExecuteInfo.nFlightFrameIndex;
		sDrawInfo.nDepthBuffer = sExecuteInfo.nDepthBufferID;
		sDrawInfo.vecRenderTargets = sExecuteInfo.vecRenderTarget;
		sDrawInfo.pRenderView = sExecuteInfo.pRenderView;

		if (!sExecuteInfo.vecSemSignal.empty())
		{
			sDrawInfo.vecSemSignal = sExecuteInfo.vecSemSignal;
		}

		if (!sExecuteInfo.vecSemWait.empty())
		{
			sDrawInfo.vecSemWait = sExecuteInfo.vecSemWait;
			sDrawInfo.vecSemWaitMasks = sExecuteInfo.vecSemWaitMask;
		}

		if (sExecuteInfo.fenceSet)
		{
			sDrawInfo.vkFenceSet = sExecuteInfo.fenceSet;
		}

		for (auto* pRenderEntity : sExecuteInfo.vecRenderEntity)
		{
			pRenderEntity->Draw(sDrawInfo);
		}
	}
} // namespace LT