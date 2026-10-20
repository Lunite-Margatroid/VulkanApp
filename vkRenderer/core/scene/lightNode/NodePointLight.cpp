// 点光源节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodePointLight.hpp"

namespace LT {
	NodePointLight::NodePointLight(NodeID id)
		: NodeLight(id), m_fRange(1.f)
	{
	}

	void NodePointLight::SetRange(float fRange) {
		m_fRange = fRange;
	}

	float NodePointLight::GetRange() const {
		return m_fRange;
	}

	void NodePointLight::FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) {
		FillLightCommon(pOutBuffer, LightType::eLightPoint);
		FillLightPosition(pOutBuffer);
		pOutBuffer->attenuation0 = m_fRange;
	}
} // namespace LT
