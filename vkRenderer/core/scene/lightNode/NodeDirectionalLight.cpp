// 平行光节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeDirectionalLight.hpp"

namespace LT {
	NodeDirectionalLight::NodeDirectionalLight(NodeID id)
		: NodeLight(id)
	{
	}

	void NodeDirectionalLight::FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) {
		FillLightCommon(pOutBuffer, LightType::eLightDirectional);
		FillLightDirection(pOutBuffer);
	}
} // namespace LT
