// Stage基类
#pragma once
#include "DeviceImage.h"
#include "EntityRender.hpp"
#include "renderView/RenderView.hpp"

namespace LT {
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

		vk::Fence fenceSet;

		StageExecuteInfo() 
			: nWidth(0)
			, nHeight(0)
			, nFlightFrameIndex(-1)
			, nDepthBufferID(INVALID_IMAGE_ID)
			, pRenderView(nullptr)
		{}
	};

	class IRenderStage {
	public:
		IRenderStage() = default;
		virtual ~IRenderStage() = default;
	

		virtual RenderStageType GetRenderStageType() const = 0;
		virtual void Execute(const StageExecuteInfo& sExecuteInfo) = 0;
	};

} // namespace LT