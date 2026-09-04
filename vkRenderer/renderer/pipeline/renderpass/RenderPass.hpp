// 编码utf-8
#pragma once

#include "VertexBuffer.h"
#include "DeviceImage.h"
#include "vkRendererUtil.hpp"

#include "ShaderModuleInfo.hpp"

namespace LT {

	// 生成着色器使用的FlagBits
	// 0-10给顶点数据使用 enum VertexChannel

	constexpr uint32_t PRIMITIVE_BIT_OFFSET = 13;
	constexpr uint32_t POLYGONMODE_BIT_OFFSET = 17;
	constexpr uint32_t LINE_WIDTH_BIT_OFFSET = 19;

	using RenderPassFlag = uint64_t;
	enum class RenderPassFlagBits : RenderFlagType {
		eBackCull = (1ull << 11), // 11 背面剔除
		eClockwise = (1ull << 12), // 12 顺时针为正面
		ePrimitiveMask = (15ull << PRIMITIVE_BIT_OFFSET), // 13-16 记录图元类型
		ePolygonModeMask = (3ull << POLYGONMODE_BIT_OFFSET), // 17-18记录填充类型
		eLineWidthMask = (15ull << LINE_WIDTH_BIT_OFFSET), // 19-22 记录线宽
		eBlendEnable = (1ull << 23), // 开启混合
	};

	inline void SetPrimitiveTopology(RenderPassFlag& nFlag, vk::PrimitiveTopology ePrimitiveTopology) {

		util::SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::ePrimitiveMask), static_cast<uint64_t>(ePrimitiveTopology) << PRIMITIVE_BIT_OFFSET);
	}

	inline vk::PrimitiveTopology GetPrimitiveTopology(const RenderPassFlag& nFlag) {
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

		return static_cast<vk::PrimitiveTopology>((static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::ePrimitiveMask)) >> PRIMITIVE_BIT_OFFSET);
	}

	inline void SetPolygonMode(RenderPassFlag& nFlag, vk::PolygonMode ePolygonMode) {
		util::SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::ePolygonModeMask), static_cast<uint64_t>(ePolygonMode) << POLYGONMODE_BIT_OFFSET);
	}

	inline vk::PolygonMode GetPolygonMode(const RenderPassFlag& nFlag) {
		static_assert(static_cast<int>(vk::PolygonMode::eFill) == 0);
		static_assert(static_cast<int>(vk::PolygonMode::eLine) == 1);
		static_assert(static_cast<int>(vk::PolygonMode::ePoint) == 2);
		return static_cast<vk::PolygonMode>((static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::ePolygonModeMask)) >> POLYGONMODE_BIT_OFFSET);
	}

	inline void SetLineWidth(RenderPassFlag& nFlag, float fLineWidth) {
		uint64_t nLineWidth = static_cast<uint64_t>(fLineWidth);

		util::SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eLineWidthMask), nLineWidth << LINE_WIDTH_BIT_OFFSET);
	}

	inline float GetLineWidth(const RenderPassFlag& nFlag) {
		return static_cast<float>((static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eLineWidthMask)) >> LINE_WIDTH_BIT_OFFSET );
	}

	inline bool IsBackCull(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag)  & static_cast<uint64_t>(RenderPassFlagBits::eBackCull);
	}

	inline void SetBackCull(RenderPassFlag& nFlag, bool bEnable) {
		if (bEnable)
		{
			util::SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBackCull));
		}
		else
		{
			util::ResetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBackCull));
		}
	}

	inline bool IsClockwiseFront(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eClockwise);
	}
	inline void SetClockwiseFront(RenderPassFlag& nFlag, bool bEnable) {
		if (bEnable)
		{
			util::SetBit(nFlag , static_cast<uint64_t>(RenderPassFlagBits::eClockwise));
		} 
		else
		{
			util::ResetBit(nFlag , static_cast<uint64_t>(RenderPassFlagBits::eClockwise));
		}
	}
	

	inline bool IsBlendEnabled(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eBlendEnable);
	}

	inline void SetBlendEnable(RenderPassFlag& nFlag, bool bEnable) {
		if (bEnable)
			util::SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBlendEnable));
		else
			util::ResetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBlendEnable));
	}

	inline bool HasVertexPos(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Position);
	}

	inline bool HasNormal(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Normal);
	}

	inline bool HasTangent(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Tangent);
	}

	inline bool HasBitangent(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Bitangent);
	}

	inline bool HasAO(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::AO);
	}

	inline bool HasColor(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(VertexChannel::Color);
	}

	inline bool HasUV(const RenderPassFlag& nFlag, uint32_t nUVChannelIndex) {
		if (nUVChannelIndex >= VERTEX_UV_CHANNEL_COUNT)
		{
			return false;
		}
		return static_cast<uint64_t>(nFlag) & (static_cast<uint64_t>(VertexChannel::UV) << nUVChannelIndex);
	}


	inline vk::ShaderStageFlags GetShaderStageFlag(BindingSpace eSpace)
	{
		switch (eSpace)
		{
			case BindingSpace::eVertexShader:
				return vk::ShaderStageFlagBits::eVertex;
			case BindingSpace::eFragmentShader:
				return vk::ShaderStageFlagBits::eFragment;
			case BindingSpace::eVertAndFragShader:
				return (vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
			default:
				return vk::ShaderStageFlagBits::eAll;
		}
	}

	inline std::string GetRenderPassDescString(const RenderPassFlag& nFlag) {
		std::stringstream oss;

		oss << "VertexLayout: \n";
		if (HasVertexPos(nFlag))
			oss << "float3\tPos\n";

		for (int i = 0; i < 5; i++)
		{
			if (HasUV(nFlag, i))
			{
				oss << (i > 1 ? "float3" : "float2") << "\t UV" << i << std::endl;
			}
		}

		if(HasNormal(nFlag))
			oss << "float3\tNormal\n";

		if(HasTangent(nFlag))
			oss << "float3\tTangent\n";

		if (HasBitangent(nFlag))
			oss << "float3\tBitangent\n";

		if (HasColor(nFlag))
			oss << "float4\tColor\n";

		if (HasAO(nFlag))
			oss << "float \tAO\n";

		oss << "Rasterization State: \n";

		oss << "lineWidth: " << GetLineWidth(nFlag) << std::endl;

		oss << "Primitive: " << static_cast<int64_t>(GetPrimitiveTopology(nFlag)) << std::endl;

		oss << "PolygonMode: " << static_cast<int64_t>(GetPolygonMode(nFlag)) << std::endl;

		oss << "BackCull: " << (IsBackCull(nFlag) ? "true" : "false") << std::endl;

		oss << "FrontFace: " << (IsClockwiseFront(nFlag) ? "clockwise" : "counterclockwise") << std::endl;

		oss << "BlendEnabled: " << (IsBlendEnabled(nFlag) ? "enable" : "disable") << std::endl;

		return oss.str();

	}

	std::vector<std::pair<const char* , const char*>> GenGraphicPPMacroDesc(const RenderPassFlag& nFlag);


	struct TransitionImageLayoutInfo {
		ImageID nImageID;
		vk::CommandBuffer vkCommandBuffer;
		vk::ImageLayout eOldLayout;
		vk::ImageLayout eNewLayout;
		vk::PipelineStageFlags2 srcStageMask;
		vk::PipelineStageFlags2 dstStageMask;
		vk::AccessFlags2 srcAccessFlag;
		vk::AccessFlags2 dstAccessFlag;
		vk::ImageAspectFlags eImageAspect;
		TransitionImageLayoutInfo()
			:nImageID(INVALID_IMAGE_ID)		
		{
			
		}
	};

	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

	public:
		static void RecordTransitionImageLayout(const TransitionImageLayoutInfo& sInfo);
	};


} // namespace LT