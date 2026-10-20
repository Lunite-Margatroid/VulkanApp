// 点光源节点

#pragma once

#include "NodeLight.hpp"

namespace LT {
	// 点光源 从所在位置向所有方向均匀发光 光照强度随距离衰减
	class NodePointLight : public NodeLight {
		friend class SceneManager;

	protected:
		float m_fRange; // 光照影响范围 超出该距离光照衰减为0

		NodePointLight(NodeID id);
		virtual ~NodePointLight() = default;

	public:
		void SetRange(float fRange);
		float GetRange() const;

		virtual LightType GetLightType() const override { return LightType::eLightPoint; }
		virtual NodeType GetNodeType() const override { return NodeType::eNodePointLight; }
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) override;
	};
} // namespace LT
