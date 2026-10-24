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
	{

	}

	Pipeline::~Pipeline() {
	}

} //namespace LT