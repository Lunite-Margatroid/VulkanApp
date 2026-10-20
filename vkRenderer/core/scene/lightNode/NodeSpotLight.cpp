// 聚光灯节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeSpotLight.hpp"

namespace LT {
	NodeSpotLight::NodeSpotLight(NodeID id)
		: NodeLight(id)
		, m_fRange(1.f)
		, m_fRadianOuter(glm::radians(45.f))
		, m_fRadianInner(glm::radians(30.f))
	{
	}

	void NodeSpotLight::SetRange(float fRange) {
		m_fRange = fRange;
	}

	float NodeSpotLight::GetRange() const {
		return m_fRange;
	}

	void NodeSpotLight::SetOuterRadian(float fRadian) {
		RENDERER_ASSERT(fRadian >= m_fRadianInner, "NodeSpotLight::SetOuterRadian: 外锥角不能小于内锥角.");
		m_fRadianOuter = fRadian;
	}

	float NodeSpotLight::GetOuterRadian() const {
		return m_fRadianOuter;
	}

	void NodeSpotLight::SetInnerRadian(float fRadian) {
		RENDERER_ASSERT(fRadian <= m_fRadianOuter, "NodeSpotLight::SetInnerRadian: 内锥角不能大于外锥角.");
		m_fRadianInner = fRadian;
	}

	float NodeSpotLight::GetInnerRadian() const {
		return m_fRadianInner;
	}

	void NodeSpotLight::FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) {
		FillLightCommon(pOutBuffer, LightType::eLightSpot);
		FillLightPosition(pOutBuffer);
		FillLightDirection(pOutBuffer);
		pOutBuffer->attenuation0 = m_fRange;
		pOutBuffer->radianOuter = m_fRadianOuter;
		pOutBuffer->radianInner = m_fRadianInner;
	}
} // namespace LT
