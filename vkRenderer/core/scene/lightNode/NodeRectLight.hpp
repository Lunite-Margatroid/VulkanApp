// 矩形面光源节点

#pragma once

#include "NodeLight.hpp"

namespace LT {
	// 矩形面光源 从所在位置沿方向发射矩形区域的光
	class NodeRectLight : public NodeLight {
		friend class SceneManager;

	protected:
		float m_fRectU; // 矩形在U方向(右方向)的半边尺寸
		float m_fRectV; // 矩形在V方向(上方向)的半边尺寸

		NodeRectLight(NodeID id);
		virtual ~NodeRectLight() = default;

	public:
		void SetRectU(float fRectU);
		float GetRectU() const;
		void SetRectV(float fRectV);
		float GetRectV() const;

		virtual LightType GetLightType() const override { return LightType::eLightRect; }
		virtual NodeType GetNodeType() const override { return NodeType::eNodeRectLight; }
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) override;
	};
} // namespace LT
