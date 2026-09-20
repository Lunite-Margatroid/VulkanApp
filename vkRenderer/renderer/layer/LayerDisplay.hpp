#pragma once
#include "Layer.hpp"

namespace LT {
	class LayerDisplay : public Layer {
	public:
		LayerDisplay();
		virtual ~LayerDisplay() = default;

		// 三五法则:删除拷贝和移动
		LayerDisplay(const LayerDisplay&) = delete;
		LayerDisplay& operator=(const LayerDisplay&) = delete;
		LayerDisplay(LayerDisplay&&) = delete;
		LayerDisplay& operator=(LayerDisplay&&) = delete;
	};
} // namespace LT
