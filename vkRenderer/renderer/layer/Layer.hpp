#pragma once

namespace LT {
	class Layer {
	public:
		Layer();
		virtual ~Layer() = default;

		// 三五法则:删除拷贝和移动
		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;
		Layer(Layer&&) = delete;
		Layer& operator=(Layer&&) = delete;
	};
} // namespace LT
