#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "LayerScene.hpp"
#include "PipelineShade.hpp"

namespace LT {
	LayerScene::LayerScene() {
		RegisterPipeline<PipelineShade>();
	}

	void LayerScene::Draw()
	{
		
	}


} // namespace LT