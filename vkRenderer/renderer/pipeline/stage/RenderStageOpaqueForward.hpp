// 不透明物体前向渲染
#pragma once
#include "IRenderStage.hpp"
#include "TransitionPass.hpp"

namespace LT {
	class RenderStageOpaqueForward : public IRenderStage
	{
	private:
		// Transiton to attachment
		std::array<vk::Semaphore, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemTransitionBegin;
		// transition to present
		std::array<vk::Semaphore, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemTransitionEnd;

		TransitionPass m_passTransition;

		// 转换为ColorAttachment
		TransitionImageLayoutInfo m_sTransitionColorToDraw;
		// 转换为DepthAttachment
		TransitionImageLayoutInfo m_sTransitionDepthToDraw;
		// 转换为Present Color
		TransitionImageLayoutInfo m_sTransitionColorToPresent;

	public:
		RenderStageOpaqueForward();
		~RenderStageOpaqueForward();

		void Execute(StageExecuteInfo& sInOutExecuteInfo) override;

		RenderStageType GetRenderStageType() const override { return RenderStageType::eOpaqueForward; }
	private:
		// 主要进行Transition
		void StageBegin(StageBeginInfo& sInOutBeginInfo) override;
		// 主要进行Transition
		void StageEnd(StageEndInfo& sInOutEndInfo) override;

		void DrawBegin(const StageDrawBeginInfo& sBeginInfo);

		void DrawEnd(StageDrawEndInfo& sEndInfo);
	};
} // namespace LT