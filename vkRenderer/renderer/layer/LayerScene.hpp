#include "Layer.hpp"

namespace LT {
	class LayerScene :public Layer{
	private:

	public:
		LayerScene();
		~LayerScene() = default;

		// 暂时没用
		void Draw() override;
	};
} // namespace LT