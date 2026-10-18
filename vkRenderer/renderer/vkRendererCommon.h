#pragma once 
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <stack>
#include <queue>
#include <chrono>
#include <ctime>
#include <optional>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <array>
#include <unordered_map>
#include <map>
#include <set>
#include <functional>
#include <variant>
#include <bitset>

#include <algorithm>

#if defined _WIN32

#define NOMINMAX
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#else

using BYTE = uint8_t;


#endif

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "TypeDef.hpp"

#include "MaterialPropDef.hpp"

#include "logger.hpp"

namespace LT {

	constexpr unsigned int RENDERER_DEFAULT_FLIGHT_FRAME_NUM = 2u;
	constexpr unsigned int RENDERER_COMMAND_BUFFER_COUNT = 4u;
	constexpr unsigned int RENDERER_DESCRIPTOR_SET_COUNT = 4u;
	enum class RenderStageType : int {
		eUnknown = -1,
		eOpaqueForward,
	};

	constexpr uint32_t VERTEX_POSITION_BIT_FLAG = (1 << 0);
	constexpr uint32_t VERTEX_UV0_BIT_FLAG = (1 << 1);
	constexpr uint32_t VERTEX_UV1_BIT_FLAG = (1 << 2);
	constexpr uint32_t VERTEX_UV2_BIT_FLAG = (1 << 3);
	constexpr uint32_t VERTEX_UV3_BIT_FLAG = (1 << 4);
	constexpr uint32_t VERTEX_UV4_BIT_FLAG = (1 << 5);
	constexpr uint32_t VERTEX_NORMAL_BIT_FLAG = (1 << 6);
	constexpr uint32_t VERTEX_TANGENT_BIT_FLAG = (1 << 7);
	constexpr uint32_t VERTEX_BITANGENT_BIT_FLAG = (1 << 8);
	constexpr uint32_t VERTEX_COLOR_BIT_FLAG = (1 << 9);
	constexpr uint32_t VERTEX_AO_BIT_FLAG = (1 << 10);

	constexpr uint32_t VERTEX_UV_CHANNEL_COUNT = 5;
	constexpr uint32_t VERTEX_CHANNEL_TOTAL_COUNT = 11;
	constexpr const char* VERTEX_CHANNEL_NAME[] = {
		"VERT_POSITION",
		"VERT_UV0",
		"VERT_UV1",
		"VERT_UV2",
		"VERT_UV3",
		"VERT_UV4",
		"VERT_NORMAL",
		"VERT_TANGENT",
		"VERT_BITANGENT",
		"VERT_COLOR",
		"VERT_AO"
	};

	constexpr uint32_t VERTEX_DIMENSION[] = {
		3, // position
		2,2,3,3,3, // UVs
		3, // normal
		3, // tangent
		3, // bitangent
		4, // color
		1  // AO
	};

	constexpr RenderFlagType VERTEX_CHANNEL_FLAG_MASK = (1 << VERTEX_CHANNEL_TOTAL_COUNT) - 1;

	// Position float3	VERT_POSITION
	// UV0 float2	VERT_UV0
	// UV1 float2	VERT_UV1
	// UV2 float3	VERT_UV2
	// UV3 float3	VERT_UV3
	// UV4 float3	VERT_UV4
	// Normal float3	VERT_NORMAL
	// Tangent float3	VERT_TANGENT
	// Bitangent float3	VERT_BITANGENT
	// Color float4	VERT_COLOR
	// AO float	VERT_AO

	enum class VertexChannel {
		Position = VERTEX_POSITION_BIT_FLAG,
		UV = VERTEX_UV0_BIT_FLAG,
		Normal = VERTEX_NORMAL_BIT_FLAG,
		Tangent = VERTEX_TANGENT_BIT_FLAG,
		Bitangent = VERTEX_BITANGENT_BIT_FLAG,
		Color = VERTEX_COLOR_BIT_FLAG,
		AO = VERTEX_AO_BIT_FLAG,
		VertexChannelMask = VERTEX_CHANNEL_FLAG_MASK
	};

	using VertexChannelFlag = uint32_t;

	template<typename T, typename MaskType>
	inline void SetBit(T& nFlag, MaskType mask) {
		nFlag |= (mask);
	}

	template<typename T, typename MaskType>
	inline void ResetBit(T& nFlag, MaskType mask) {
		nFlag &= (~(mask));
	}

	template<typename T, typename MaskType>
	inline void SetBit(T& nFlag, MaskType nMask, T nValue) {
		nFlag = (nFlag & ~nMask) | (nValue & nMask);
	}



// 生成着色器使用的FlagBits
// 0-10给顶点数据使用 enum VertexChannel
// 32-47给材质属性贴图使用 MTL_PROP_TEX_MAX_COUNT个槽位 每个bit对应一个贴图槽位

	constexpr uint32_t PRIMITIVE_BIT_OFFSET = 13u;
	constexpr uint32_t POLYGONMODE_BIT_OFFSET = 17u;
	constexpr uint32_t LINE_WIDTH_BIT_OFFSET = 19u;
	constexpr uint32_t MTL_PROP_TEX_MAX_COUNT = 16u;
	constexpr uint32_t MTL_PROP_TEX0_BIT_OFFSET = 32u;
	constexpr uint64_t MTL_PROP_TEX_MASK = ((1ull << MTL_PROP_TEX_MAX_COUNT) - 1) << MTL_PROP_TEX0_BIT_OFFSET;

	using RenderPassFlag = uint64_t;
	enum class RenderPassFlagBits : RenderFlagType {
		eBackCull = (1ull << 11), // 11 背面剔除
		eClockwise = (1ull << 12), // 12 顺时针为正面
		ePrimitiveMask = (15ull << PRIMITIVE_BIT_OFFSET), // 13-16 记录图元类型
		ePolygonModeMask = (3ull << POLYGONMODE_BIT_OFFSET), // 17-18记录填充类型
		eLineWidthMask = (15ull << LINE_WIDTH_BIT_OFFSET), // 19-22 记录线宽
		eBlendEnable = (1ull << 23), // 开启混合
		// 32-47 材质属性贴图标志位 每个bit对应一个材质属性贴图槽位
		eMtlPropTex0 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 0)),
		eMtlPropTex1 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 1)),
		eMtlPropTex2 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 2)),
		eMtlPropTex3 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 3)),
		eMtlPropTex4 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 4)),
		eMtlPropTex5 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 5)),
		eMtlPropTex6 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 6)),
		eMtlPropTex7 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 7)),
		eMtlPropTex8 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 8)),
		eMtlPropTex9 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 9)),
		eMtlPropTex10 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 10)),
		eMtlPropTex11 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 11)),
		eMtlPropTex12 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 12)),
		eMtlPropTex13 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 13)),
		eMtlPropTex14 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 14)),
		eMtlPropTex15 = (1ull << (MTL_PROP_TEX0_BIT_OFFSET + 15)),
		eMtlPropTexMask = MTL_PROP_TEX_MASK,

		eInvalid = 1ull << 63

	};

	inline bool IsValid(const RenderPassFlag& nFlag) {
		return !(static_cast<RenderFlagType>(RenderPassFlagBits::eInvalid) & static_cast<RenderFlagType>(nFlag));
	}

	inline void SetPrimitiveTopology(RenderPassFlag& nFlag, vk::PrimitiveTopology ePrimitiveTopology) {

		SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::ePrimitiveMask), static_cast<uint64_t>(ePrimitiveTopology) << PRIMITIVE_BIT_OFFSET);
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
		SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::ePolygonModeMask), static_cast<uint64_t>(ePolygonMode) << POLYGONMODE_BIT_OFFSET);
	}

	inline vk::PolygonMode GetPolygonMode(const RenderPassFlag& nFlag) {
		static_assert(static_cast<int>(vk::PolygonMode::eFill) == 0);
		static_assert(static_cast<int>(vk::PolygonMode::eLine) == 1);
		static_assert(static_cast<int>(vk::PolygonMode::ePoint) == 2);
		return static_cast<vk::PolygonMode>((static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::ePolygonModeMask)) >> POLYGONMODE_BIT_OFFSET);
	}

	inline void SetLineWidth(RenderPassFlag& nFlag, float fLineWidth) {
		uint64_t nLineWidth = static_cast<uint64_t>(fLineWidth);

		SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eLineWidthMask), nLineWidth << LINE_WIDTH_BIT_OFFSET);
	}

	inline float GetLineWidth(const RenderPassFlag& nFlag) {
		return static_cast<float>((static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eLineWidthMask)) >> LINE_WIDTH_BIT_OFFSET);
	}

	inline bool IsBackCull(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eBackCull);
	}

	inline void SetBackCull(RenderPassFlag& nFlag, bool bEnable) {
		if (bEnable)
		{
			SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBackCull));
		}
		else
		{
			ResetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBackCull));
		}
	}

	inline bool IsClockwiseFront(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eClockwise);
	}
	inline void SetClockwiseFront(RenderPassFlag& nFlag, bool bEnable) {
		if (bEnable)
		{
			SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eClockwise));
		}
		else
		{
			ResetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eClockwise));
		}
	}


	inline bool IsBlendEnabled(const RenderPassFlag& nFlag) {
		return static_cast<uint64_t>(nFlag) & static_cast<uint64_t>(RenderPassFlagBits::eBlendEnable);
	}

	inline void SetBlendEnable(RenderPassFlag& nFlag, bool bEnable) {
		if (bEnable)
			SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBlendEnable));
		else
			ResetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eBlendEnable));
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

	inline bool HasMtlPropTex(const RenderPassFlag& nFlag, uint32_t nTexSlotIndex) {
		if (nTexSlotIndex >= MTL_PROP_TEX_MAX_COUNT)
		{
			return false;
		}
		return static_cast<uint64_t>(nFlag) & (static_cast<uint64_t>(RenderPassFlagBits::eMtlPropTex0) << nTexSlotIndex);
	}

	inline void SetMtlPropTex(RenderPassFlag& nFlag, uint32_t nTexSlotIndex, bool bEnable) {
		if (nTexSlotIndex >= MTL_PROP_TEX_MAX_COUNT)
		{
			return;
		}
		if (bEnable)
		{
			SetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eMtlPropTex0) << nTexSlotIndex);
		}
		else
		{
			ResetBit(nFlag, static_cast<uint64_t>(RenderPassFlagBits::eMtlPropTex0) << nTexSlotIndex);
		}
	}
} // namespace LT
