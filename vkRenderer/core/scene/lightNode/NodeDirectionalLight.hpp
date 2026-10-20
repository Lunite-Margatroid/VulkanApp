// 平行光节点

#pragma once

#include "NodeLight.hpp"

namespace LT {
	// 平行光 光照方向固定且强度不随距离衰减 方向由节点旋转决定
	class NodeDirectionalLight : public NodeLight {
		friend class SceneManager;

	protected:
		NodeDirectionalLight(NodeID id);
		virtual ~NodeDirectionalLight() = default;

	public:
		virtual LightType GetLightType() const override { return LightType::eLightDirectional; }
		virtual NodeType GetNodeType() const override { return NodeType::eNodeDirectionalLight; }
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) override;
	};
} // namespace LT
