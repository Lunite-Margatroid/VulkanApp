#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeLight.hpp"

namespace LT {
	NodeLight::NodeLight(NodeID id)
		: Node(id), m_cColor({ 1.f, 1.f, 1.f }), m_bitsLightGroup(1ull) // 默认最低位为1 灯光默认在灯光组0内
	{
	}

	void NodeLight::SetLightColor(float r, float g, float b) {
		m_cColor = RGB_32{ r, g, b };
	}

	void NodeLight::SetLightColor(const RGB_32& cColor) {
		m_cColor = cColor;
	}

	const RGB_32& NodeLight::GetLightColor() const {
		return m_cColor;
	}

	bool NodeLight::IsInLightGroup(uint32_t nGroupIndex) const {
		RENDERER_ASSERT(nGroupIndex < LIGHT_GROUP_MAX_COUNT, "NodeLight::IsInLightGroup: 灯光组索引超出范围.");
		return m_bitsLightGroup.test(nGroupIndex);
	}

	void NodeLight::SetLightGroup(uint32_t nGroupIndex, bool bEnable) {
		RENDERER_ASSERT(nGroupIndex < LIGHT_GROUP_MAX_COUNT, "NodeLight::SetLightGroup: 灯光组索引超出范围.");
		m_bitsLightGroup.set(nGroupIndex, bEnable);
	}

	const std::bitset<LIGHT_GROUP_MAX_COUNT>& NodeLight::GetLightGroupBits() const {
		return m_bitsLightGroup;
	}

	void NodeLight::FillLightCommon(LightUniformBuffer* pOutBuffer, LightType eLightType) {
		RENDERER_ASSERT(pOutBuffer, "NodeLight::FillLightCommon: pOutBuffer is nullptr.");
		*pOutBuffer = {};
		pOutBuffer->color = { m_cColor.r, m_cColor.g, m_cColor.b };
		pOutBuffer->nLightType = eLightType;
	}

	void NodeLight::FillLightPosition(LightUniformBuffer* pOutBuffer) {
		glm::vec3 vPos = glm::vec3(GetGlobalMat()[3]);
		pOutBuffer->position = { vPos.x, vPos.y, vPos.z };
	}

	void NodeLight::FillLightDirection(LightUniformBuffer* pOutBuffer) {
		// 方向 = 全局旋转矩阵 * -Y轴 默认朝下
		glm::vec3 vDir = glm::normalize(glm::mat3(GetGlobalMat()) * -VEC3_AXIS_Y);
		pOutBuffer->direction = { vDir.x, vDir.y, vDir.z };
	}
} // namespace LT
