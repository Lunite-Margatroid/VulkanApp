#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "LayerScene.hpp"
#include "PipelineShade.hpp"
#include "PipelineLog.hpp"

namespace LT {
	LayerScene::LayerScene() {
		RegisterPipeline<PipelineShade>();
		m_vecPipelines.back()->SetColorInType({PipelineIOType::eUndefined});
		m_vecPipelines.back()->SetColorOutType({ PipelineIOType::eColorAttachment});
		m_vecPipelines.back()->SetDepthInType(PipelineIOType::eUndefined);
		m_vecPipelines.back()->SetDepthOutType(PipelineIOType::eDepthAttachment);

		RegisterPipeline<PipelineLog>();
		m_vecPipelines.back()->SetColorInType({ PipelineIOType::eColorAttachment });
		m_vecPipelines.back()->SetColorOutType({ PipelineIOType::eColorPresent });
		m_vecPipelines.back()->SetDepthInType(PipelineIOType::eUndefined);
		m_vecPipelines.back()->SetDepthOutType(PipelineIOType::eUndefined);
	}

	void LayerScene::Draw()
	{
		
	}


} // namespace LT