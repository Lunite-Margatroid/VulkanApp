// 不透明物体前向渲染
#pragma once
#include "IRenderStage.hpp"

namespace LT {
	class RenderStageOpaqueForward : public IRenderStage
	{
	public:
		RenderStageOpaqueForward() = default;
		~RenderStageOpaqueForward() = default;
	};
} // namespace LT