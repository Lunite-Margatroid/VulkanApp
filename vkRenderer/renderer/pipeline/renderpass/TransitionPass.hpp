#pragma once
#include "RenderPass.hpp"
#include "FenceManager.hpp"
namespace LT {

	struct RecordTransitionInfo {
		CommandBufferWrapper* pCmdBuffer;
		std::vector<TransitionImageLayoutInfo> vecColorAttachment;
		std::vector<TransitionImageLayoutInfo> vecDepthAttachment;
	};

	struct TransitionSubmitInfo {
		FlightFrameIndex nFlightFrameIndex;

		std::vector<vk::Semaphore> vecSemToWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMasks;
		std::vector<vk::Semaphore> vecSemToSignal;
		// 当前Submit的Fence
		FenceRef refFence;

	};

	class TransitionPass : public RenderPass
	{
	private:

	public:
		TransitionPass() = default;
		~TransitionPass() = default;

		TransitionPass(TransitionPass&&) = delete;
		TransitionPass(const TransitionPass&) = delete;

		TransitionPass& operator = (TransitionPass&&) = delete;
		TransitionPass& operator = (const TransitionPass&) = delete;

		void RecordCommand(const RecordTransitionInfo& sInOutRecordInfo);
		void Submit(TransitionSubmitInfo& sInOutSubmitInfo);
	};
}// namespace LT
