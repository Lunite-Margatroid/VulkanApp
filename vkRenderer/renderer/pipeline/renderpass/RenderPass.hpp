// 编码utf-8
#pragma once

#include "VertexBuffer.h"

namespace LT {

	// 生成着色器使用的FlagBits
	// 0-10给顶点数据使用 enum VertexChannel

	constexpr uint32_t PRIMITIVE_BIT_OFFSET = 13;

	using RenderPassFlag = uint64_t;
	enum class RenderPassFlagBits : uint64_t {
		eBackCull = (1ull << 11), // 11 背面剔除
		eClockwise = (1ull << 12), // 12 顺时针为正面
		ePrimitiveMask = (15ull << PRIMITIVE_BIT_OFFSET), // 13-16 记录图元类型
	};

	static vk::PrimitiveTopology GetPrimitiveTopology(const RenderPassFlag& nFlag) {
		static_assert(static_cast<int>(vk::PrimitiveTopology::ePointList) == 0);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eLineList) == 1);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eLineStrip) == 2);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eTriangleList) == 3);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eTriangleStrip) == 4);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eTriangleFan) == 5);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eLineListWithAdjacency) == 6);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eLineStripWithAdjacency) == 7);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eTriangleListWithAdjacency) == 8);
		static_assert(static_cast<int>(vk::PrimitiveTopology::eTriangleStripWithAdjacency) == 9);
		static_assert(static_cast<int>(vk::PrimitiveTopology::ePatchList) == 10);

		return static_cast<vk::PrimitiveTopology>((static_cast<uint64_t>(nFlag) >> PRIMITIVE_BIT_OFFSET) & static_cast<uint64_t>(RenderPassFlagBits::ePrimitiveMask));
	}

	static bool IsBackCull(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag)  & static_cast<uint64_t>(RenderPassFlagBits::eBackCull);
	}

	static bool IsClockwiseFront(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eClockwise);
	}

	static bool HasVertexPos(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Position);
	}

	static bool HasNormal(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Normal);
	}

	static bool HasTangent(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Tangent);
	}

	static bool HasBitangent(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Bitangent);
	}

	static bool HasAO(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::AO);
	}

	static bool HasColor(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Color);
	}

	static bool HasUV(const RenderPassFlag& nFlag, uint32_t nUVChannelIndex) {
		if (nUVChannelIndex >= VERTEX_UV_CHANNEL_COUNT)
		{
			return false;
		}
		return static_cast<uint64_t>(nFlag) & (static_cast<uint64_t>(VertexChannel::UV) << nUVChannelIndex);
	}

	std::vector<std::pair<const char* , const char*>> GenGraphicPPMacroDesc(const RenderPassFlag& nFlag);

	class RenderPass
	{
	public:
		virtual ~RenderPass();
		virtual void Execute() = 0;
	};


} // namespace LT