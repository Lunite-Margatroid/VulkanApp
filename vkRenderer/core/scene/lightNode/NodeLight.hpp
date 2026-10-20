#pragma once
#include <bitset>
#include "Node.hpp"

namespace LT {
	// 灯光节点基类 所有灯光节点从这里派生
	// 灯光类型见TypeDef.hpp中的LightType 节点类型见TypeDef.hpp中的NodeType
	class NodeLight : public Node {
		friend class SceneManager;

	protected:
		RGB_32 m_cColor; // 灯光颜色
		// 灯光组标志位 第n位为1表示该灯光在灯光组n内
		std::bitset<LIGHT_GROUP_MAX_COUNT> m_bitsLightGroup;

		NodeLight(NodeID id);
		virtual ~NodeLight() = default;

		// 填充灯光公共字段 将buffer清零并填入颜色和灯光类型
		void FillLightCommon(LightUniformBuffer* pOutBuffer, LightType eLightType);
		// 填充灯光世界坐标位置 取全局变换矩阵的平移分量
		void FillLightPosition(LightUniformBuffer* pOutBuffer);
		// 填充灯光世界坐标方向 方向由旋转决定 默认朝向-Z轴(与Camera默认朝向一致)
		void FillLightDirection(LightUniformBuffer* pOutBuffer);

	public:
		// 获取灯光类型
		virtual LightType GetLightType() const = 0;
		// 将灯光数据填入LightUniformBuffer 供着色器使用
		virtual void FillLightUniformBuffer(LightUniformBuffer* pOutBuffer) = 0;

		void SetLightColor(float r, float g, float b);
		void SetLightColor(const RGB_32& cColor);
		const RGB_32& GetLightColor() const;

		// 判断灯光是否在灯光组nGroupIndex内
		bool IsInLightGroup(uint32_t nGroupIndex) const;
		// 设置灯光是否在灯光组nGroupIndex内
		void SetLightGroup(uint32_t nGroupIndex, bool bEnable);
		// 获取灯光组标志位
		const std::bitset<LIGHT_GROUP_MAX_COUNT>& GetLightGroupBits() const;
	};
} // namespace LT
