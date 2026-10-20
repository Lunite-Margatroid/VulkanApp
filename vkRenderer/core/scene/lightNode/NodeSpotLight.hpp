// 聚光灯节点

#pragma once

#include "NodeLight.hpp"

namespace LT {
	// 聚光灯 从所在位置沿方向发射锥形光束
	// 内锥角内光照强度恒定 内外锥角之间强度渐变到0
	class NodeSpotLight : public NodeLight {
		friend class SceneManager;

	protected:
		float m_fRange; // 光照影响范围 超出该距离光照衰减为0
		float m_fRadianOuter; // 外锥角(弧度) 超过该角度的方向无光照
		float m_fRadianInner; // 内锥角(弧度) 该角度内光照强度恒定

		NodeSpotLight(NodeID id);
		virtual ~NodeSpotLight() = default;

	public:
		void SetRange(float fRange);
		float GetRange() const;
		// 设置外锥角(弧度) 必须不小于内锥角
		void SetOuterRadian(float fRadian);
		float GetOuterRadian() const;
		// 设置内锥角(弧度) 必须不大于外锥角
		void SetInnerRadian(float fRadian);
		float GetInnerRadian() const;

		virtual LightType GetLightType() const override { return LightType::eLightSpot; }
		virtual NodeType GetNodeType() const override { return NodeType::eNodeSpotLight; }
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) override;
	};
} // namespace LT
