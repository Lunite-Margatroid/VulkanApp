// Stage基类
#pragma once
#include "DeviceImage.h"
#include "renderView/RenderView.hpp"
#include "FenceManager.hpp"

namespace LT {
	class EntityRender;

	struct StageExecuteInfo {
		uint32_t nWidth;
		uint32_t nHeight;
		FlightFrameIndex nFlightFrameIndex;
		ImageID nDepthBufferID;
		std::vector<ImageID> vecRenderTarget;
		std::vector<EntityRender*> vecRenderEntity;
		RenderView* pRenderView;

		std::vector<vk::Semaphore> vecSemWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMask;

		std::vector<vk::Semaphore> vecSemSignal;

		// 当前Stage最后一次Submit的Fence
		// 通常作为输出参数
		FenceRef refFence;

		StageExecuteInfo() 
			: nWidth(0)
			, nHeight(0)
			, nFlightFrameIndex(-1)
			, nDepthBufferID(INVALID_IMAGE_ID)
			, pRenderView(nullptr)
			, refFence(INVALID_FENCE_HANDLE)
		{}
	};

	class IRenderStage {
	public:
		IRenderStage() = default;
		virtual ~IRenderStage() = default;
	

		virtual RenderStageType GetRenderStageType() const = 0;
		virtual void Execute(StageExecuteInfo& sInOutExecuteInfo) = 0;
	};

} // namespace LT