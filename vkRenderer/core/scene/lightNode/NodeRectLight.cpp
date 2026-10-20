// 矩形面光源节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeRectLight.hpp"

namespace LT {
	NodeRectLight::NodeRectLight(NodeID id)
		: NodeLight(id), m_fRectU(1.f), m_fRectV(1.f)
	{
	}

	void NodeRectLight::SetRectU(float fRectU) {
		m_fRectU = fRectU;
	}

	float NodeRectLight::GetRectU() const {
		return m_fRectU;
	}

	void NodeRectLight::SetRectV(float fRectV) {
		m_fRectV = fRectV;
	}

	float NodeRectLight::GetRectV() const {
		return m_fRectV;
	}

	void NodeRectLight::FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) {
		FillLightCommon(pOutBuffer, LightType::eLightRect);
		FillLightPosition(pOutBuffer);
		FillLightDirection(pOutBuffer);
		pOutBuffer->rectU = m_fRectU;
		pOutBuffer->rectV = m_fRectV;
	}
} // namespace LT
