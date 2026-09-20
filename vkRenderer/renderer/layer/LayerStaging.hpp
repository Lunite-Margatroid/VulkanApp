#pragma once
#include "Layer.hpp"

namespace LT {
	class LayerStaging : public Layer {
	public:
		LayerStaging();
		virtual ~LayerStaging() = default;

		// 三五法则:删除拷贝和移动
		LayerStaging(const LayerStaging&) = delete;
		LayerStaging& operator=(const LayerStaging&) = delete;
		LayerStaging(LayerStaging&&) = delete;
		LayerStaging& operator=(LayerStaging&&) = delete;
	};
} // namespace LT
