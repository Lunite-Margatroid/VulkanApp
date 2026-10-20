// 球光源节点

#pragma once

#include "NodeLight.hpp"

namespace LT {
	// 球光源 从球体表面向所有方向发光 球体半径越大光源越大
	class NodeSphereLight : public NodeLight {
		friend class SceneManager;

	protected:
		float m_fRadius; // 光源球体半径

		NodeSphereLight(NodeID id);
		virtual ~NodeSphereLight() = default;

	public:
		void SetRadius(float fRadius);
		float GetRadius() const;

		virtual LightType GetLightType() const override { return LightType::eLightSphere; }
		virtual NodeType GetNodeType() const override { return NodeType::eNodeSphereLight; }
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) override;
	};
} // namespace LT
