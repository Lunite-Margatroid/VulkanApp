// 渲染视图
#pragma once

namespace LT {
	class RenderView {
	public:
		RenderView() = default;
		virtual ~RenderView() = default;

		// 五法则:删除拷贝和移动
		RenderView(const RenderView&) = delete;
		RenderView& operator=(const RenderView&) = delete;
		RenderView(RenderView&&) = delete;
		RenderView& operator=(RenderView&&) = delete;
	};
} // namespace LT
