// 不透明物体前向渲染
#pragma once
#include "IRenderStage.hpp"
#include "DeviceImage.h"

namespace LT {
	class RenderStageOpaqueForward : public IRenderStage
	{
	public:
		RenderStageOpaqueForward() = default;
		~RenderStageOpaqueForward() = default;

		void Execute(const StageExecuteInfo& sExecuteInfo) override;

		RenderStageType GetRenderStageType() const override { return RenderStageType::eOpaqueForward; }
	};
} // namespace LT