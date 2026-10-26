// 不透明物体前向渲染
#pragma once
#include "IRenderStage.hpp"
#include "TransitionPass.hpp"

namespace LT {
	class RenderStageOpaqueForward : public IRenderStage
	{
	private:
		// Begin Stage Transition
		std::array<vk::Semaphore, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemTransitionBegin;
		// End Stage Transition
		std::array<vk::Semaphore, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemTransitionEnd;

		TransitionPass m_passTransition;

	public:
		RenderStageOpaqueForward();
		~RenderStageOpaqueForward();

		void Execute(StageExecuteInfo& sInOutExecuteInfo) override;

		RenderStageType GetRenderStageType() const override { return RenderStageType::eOpaqueForward; }
	private:
		// 主要进行Transition
		// 如果输出的Fence是false 就是没有进行Transition 没有submit
		void StageBegin(StageBeginInfo& sInOutBeginInfo) override;
		// 主要进行Transition
		void StageEnd(StageEndInfo& sInOutEndInfo) override;

		void DrawBegin(const StageDrawBeginInfo& sBeginInfo);

		void DrawEnd(StageDrawEndInfo& sEndInfo);
	};
} // namespace LT