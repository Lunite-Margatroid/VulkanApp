// Stage基类
#pragma once
#include "DeviceImage.h"
#include "renderView/RenderView.hpp"
#include "FenceManager.hpp"

#include "TransitionPass.hpp"

namespace LT {
	class EntityRender;

	class Pipeline;

	struct StageExecuteInfo {
		uint32_t nWidth;
		uint32_t nHeight;
		FlightFrameIndex nFlightFrameIndex;
		ImageID nDepthBufferID;
		std::vector<ImageID> vecRenderTarget;
		//std::vector<EntityRender*> vecRenderEntity;
		std::map<MaterialID, std::vector<EntityRender*>> mapRenderEntity;
		RenderView* pRenderView;

		std::vector<vk::Semaphore> vecSemWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMask;

		std::vector<vk::Semaphore> vecSemSignal;

		// 当前Stage最后一次Submit的Fence
		// 通常作为输出参数
		FenceRef refFence;

		// 该Stage实例所属的Pipeline
		Pipeline* pPipeline;
		// 该Stage是否需要clear color attachment
		std::vector<bool> vecColorClear;
		bool bDepthClear;

		StageExecuteInfo() 
			: nWidth(0)
			, nHeight(0)
			, nFlightFrameIndex(-1)
			, nDepthBufferID(INVALID_IMAGE_ID)
			, pRenderView(nullptr)
			, refFence(INVALID_FENCE_HANDLE)
			, pPipeline(nullptr)
			, bDepthClear(true)
		{}
	};

	struct StageDrawBeginInfo {
		uint32_t nWidth;
		uint32_t nHeight;
		FlightFrameIndex nFlightFrameIndex;
		ImageID nDepthBufferID;
		std::vector<ImageID> vecRenderTarget;

		std::vector<vk::AttachmentLoadOp> vecColorLoadOp;
		std::vector<vk::AttachmentStoreOp> vecColorStoreOp;
		vk::AttachmentLoadOp eDepthLoadOp;
		vk::AttachmentStoreOp eDepthStoreOp;
		StageDrawBeginInfo() 
			: nWidth(0)
			, nHeight(0)
			, nFlightFrameIndex(-1)
			, nDepthBufferID(INVALID_IMAGE_ID)
			, eDepthLoadOp(vk::AttachmentLoadOp::eLoad)
			, eDepthStoreOp(vk::AttachmentStoreOp::eDontCare)
		{}
	};

	struct StageDrawEndInfo {
		FlightFrameIndex nFlightFrameIndex;

		// 这里面有Submit 需要时序控制
		std::vector<vk::Semaphore> vecSemWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMask;

		std::vector<vk::Semaphore> vecSemSignal;

		// 当前Stage最后一次Submit的Fence
		// 通常作为输出参数
		FenceRef refFence;
		StageDrawEndInfo() :nFlightFrameIndex(-1){}
	};

	struct StageBeginInfo {
		FlightFrameIndex nFlightFrameIndex;
		// 需要Transition的Image
		ImageID nDepthBufferID;
		std::vector<ImageID> vecRenderTarget;
		// 需要等待的Semaphore
		std::vector<vk::Semaphore> vecSemWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMask;
		// 发射的Semaphore
		std::vector<vk::Semaphore> vecSemSignal;
		// 当前Submit的Fence
		FenceRef refFence;
		StageBeginInfo() 
			: nFlightFrameIndex(0)
			, nDepthBufferID(INVALID_IMAGE_ID)
			, refFence(INVALID_FENCE_HANDLE)
		{}
	};

	using StageEndInfo = StageBeginInfo;

	class IRenderStage {
	protected:
		
		// 标识Transition类型
		std::vector<PipelineIOType> m_vecColorInType;
		std::vector<PipelineIOType> m_vecColorOutType;
		PipelineIOType m_eDepthInType;
		PipelineIOType m_eDepthOutType;

		// Begin Stage Transition
		std::array<vk::Semaphore, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemTransitionBegin;
		// End Stage Transition
		std::array<vk::Semaphore, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemTransitionEnd;

		TransitionPass m_passTransition;

	public:
		IRenderStage();
		virtual ~IRenderStage();
	

		virtual RenderStageType GetRenderStageType() const = 0;
		virtual void Execute(StageExecuteInfo& sInOutExecuteInfo) = 0;

		// 主要进行Transition
		// 如果输出的Fence是false 就是没有进行Transition 没有submit
		void StageBegin(StageBeginInfo& sInOutBeginInfo);
		// 主要进行Transition
		void StageEnd(StageEndInfo& sInOutEndInfo);

		virtual bool OutNeedTransiton(std::span<ImageID> vecColorAttachments) const;
		//virtual bool InNeedTransiton() const;

		// 输入的颜色缓冲的类型
		void SetColorInType(const std::vector<PipelineIOType>& vecColorInType);
		// 输出的颜色缓冲的类型
		void SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType);
		// 输入的深度缓冲的类型
		void SetDepthInType(PipelineIOType eDepthInType);
		// 输出的深度缓冲的类型
		void SetDepthOutType(PipelineIOType eDepthOutType);
	};

} // namespace LT