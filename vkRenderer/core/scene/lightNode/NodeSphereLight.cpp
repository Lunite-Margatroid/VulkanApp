// 球光源节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeSphereLight.hpp"

namespace LT {
	NodeSphereLight::NodeSphereLight(NodeID id)
		: NodeLight(id), m_fRadius(1.f)
	{
	}

	void NodeSphereLight::SetRadius(float fRadius) {
		m_fRadius = fRadius;
	}

	float NodeSphereLight::GetRadius() const {
		return m_fRadius;
	}

	void NodeSphereLight::FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) {
		FillLightCommon(pOutBuffer, LightType::eLightSphere);
		FillLightPosition(pOutBuffer);
		pOutBuffer->radius = m_fRadius;
	}
} // namespace LT
