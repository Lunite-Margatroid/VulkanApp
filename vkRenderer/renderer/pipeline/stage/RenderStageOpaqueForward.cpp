// 不透明物体前向渲染
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "RenderStageOpaqueForward.hpp"

#include "EntityRender.hpp"

namespace LT {
	void RenderStageOpaqueForward::Execute(StageExecuteInfo& sInOutExecuteInfo)
	{
		if (sInOutExecuteInfo.vecRenderEntity.size() <= 0)
			return;

		EntityDrawInfo sDrawInfo;
		sDrawInfo.nWidth = sInOutExecuteInfo.nWidth;
		sDrawInfo.nHeight = sInOutExecuteInfo.nHeight;
		sDrawInfo.nFlightFrameIndex = sInOutExecuteInfo.nFlightFrameIndex;
		sDrawInfo.nDepthBuffer = sInOutExecuteInfo.nDepthBufferID;
		sDrawInfo.vecRenderTargets = sInOutExecuteInfo.vecRenderTarget;
		sDrawInfo.pRenderView = sInOutExecuteInfo.pRenderView;
		sDrawInfo.eRenderStage = GetRenderStageType();
		sDrawInfo.bClear = true;

		if (sInOutExecuteInfo.vecRenderEntity.size() >= 2)
		{
			// 第一个渲染实体
			if (!sInOutExecuteInfo.vecSemWait.empty())
			{
				sDrawInfo.vecSemWait = sInOutExecuteInfo.vecSemWait;
				sDrawInfo.vecSemWaitMasks = sInOutExecuteInfo.vecSemWaitMask;
			}
			sInOutExecuteInfo.vecRenderEntity.front()->Draw(sDrawInfo);

			sDrawInfo.vecSemWait.clear();
			sDrawInfo.vecSemWaitMasks.clear();

			sDrawInfo.bClear = false;
			// 第2到n个渲染实体
			for (int i = 1; i < sInOutExecuteInfo.vecRenderEntity.size() - 1; i++)
			{
				sInOutExecuteInfo.vecRenderEntity[i]->Draw(sDrawInfo);
			}
			// 最后一个渲染实体
			if (!sInOutExecuteInfo.vecSemSignal.empty())
			{
				sDrawInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
			}

			sInOutExecuteInfo.vecRenderEntity.back()->Draw(sDrawInfo);
			sInOutExecuteInfo.refFence = sDrawInfo.refFence;
		}
		else if(sInOutExecuteInfo.vecRenderEntity.size() > 0)
		{
			if (!sInOutExecuteInfo.vecSemWait.empty())
			{
				sDrawInfo.vecSemWait = sInOutExecuteInfo.vecSemWait;
				sDrawInfo.vecSemWaitMasks = sInOutExecuteInfo.vecSemWaitMask;
			}
			if (!sInOutExecuteInfo.vecSemSignal.empty())
			{
				sDrawInfo.vecSemSignal = sInOutExecuteInfo.vecSemSignal;
			}
			sInOutExecuteInfo.vecRenderEntity.front()->Draw(sDrawInfo);
			sInOutExecuteInfo.refFence = sDrawInfo.refFence;
		}

	}
} // namespace LT