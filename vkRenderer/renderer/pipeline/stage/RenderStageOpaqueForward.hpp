// 不透明物体前向渲染
#pragma once
#include "IRenderStage.hpp"

namespace LT {
	class RenderStageOpaqueForward : public IRenderStage
	{

	public:
		RenderStageOpaqueForward();
		~RenderStageOpaqueForward();

		void Execute(StageExecuteInfo& sInOutExecuteInfo) override;

		RenderStageType GetRenderStageType() const override { return RenderStageType::eOpaqueForward; }
	private:


		void DrawBegin(const StageDrawBeginInfo& sBeginInfo);

		void DrawEnd(StageDrawEndInfo& sEndInfo);
	};
} // namespace LT