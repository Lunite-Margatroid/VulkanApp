// 环境光节点

#pragma once

#include "NodeLight.hpp"

namespace LT {
	// 环境光 均匀照亮整个场景 与位置和方向无关
	class NodeAmbient : public NodeLight {
		friend class SceneManager;

	protected:
		NodeAmbient(NodeID id);
		virtual ~NodeAmbient() = default;

	public:
		virtual LightType GetLightType() const override { return LightType::eLightAmbient; }
		virtual NodeType GetNodeType() const override { return NodeType::eNodeAmbient; }
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) override;
	};
} // namespace LT
