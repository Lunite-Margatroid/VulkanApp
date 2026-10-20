// 环境光节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeAmbient.hpp"

namespace LT {
	NodeAmbient::NodeAmbient(NodeID id)
		: NodeLight(id)
	{
	}

	void NodeAmbient::FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) {
		FillLightCommon(pOutBuffer, LightType::eLightAmbient);
	}
} // namespace LT
