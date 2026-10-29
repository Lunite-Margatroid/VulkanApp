// Stage基类
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "IRenderStage.hpp"

namespace LT {
	IRenderStage::IRenderStage() 
	: m_eDepthInType(PipelineIOType::eUndefined)
	, m_eDepthOutType(PipelineIOType::eDepthAttachment)
	{}

	void IRenderStage::SetColorInType(const std::vector<PipelineIOType>& vecColorInType)
	{
		m_vecColorInType = vecColorInType;
	}

	void IRenderStage::SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType)
	{
		m_vecColorOutType = vecColorOutType;
	}

	void IRenderStage::SetDepthInType(PipelineIOType eDepthInType)
	{
		m_eDepthInType = eDepthInType;
	}

	void IRenderStage::SetDepthOutType(PipelineIOType eDepthOutType)
	{
		m_eDepthOutType = eDepthOutType;
	}

} // namespace 