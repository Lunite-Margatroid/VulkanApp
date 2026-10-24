#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "Layer.hpp"
#include "Pipeline.hpp"

namespace LT {
	Layer::Layer()
	{
	}

	Layer::~Layer()
	{
		for (Pipeline* pPipeline : m_vecPipelines)
		{
			delete pPipeline;
		}
		m_vecPipelines.clear();
	}

	const std::vector<Pipeline*>& Layer::GetPipelines() const {
		return m_vecPipelines;
	}

} // namespace LT
