#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "SwapChain.h"
#include "Pipeline.hpp"
#include "IBindable.h"

#include "ImageSampler.h"


namespace LT {
	Pipeline::Pipeline()
		:m_nWidth(DEFAULT_WIDTH)
		,m_nHeight(DEFAULT_HEIGHT)
		,m_eDepthInType(PipelineIOType::eDepthAttachment)
		,m_eDepthOutType(PipelineIOType::eDepthAttachment)
	{
		m_vecColorInType.push_back(PipelineIOType::eColorAttachment);
		m_vecColorOutType.push_back(PipelineIOType::eColorAttachment);

	}

	Pipeline::~Pipeline() {
	}

	void Pipeline::SetColorInType(const std::vector<PipelineIOType>& vecColorInType)
	{
		m_vecColorInType = vecColorInType;
	}

	void Pipeline::SetColorOutType(const std::vector<PipelineIOType>&vecColorOutType)
	{
		m_vecColorOutType = vecColorOutType;
	}

	void Pipeline::SetDepthInType(PipelineIOType eDepthInType)
	{
		m_eDepthInType = eDepthInType;
	}

	void Pipeline::SetDepthOutType(PipelineIOType eDepthOutType)
	{
		m_eDepthOutType = eDepthOutType;
	}

} //namespace LT