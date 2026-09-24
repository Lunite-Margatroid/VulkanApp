// 编码utf-8
#pragma once

#include "VertexBuffer.h"
#include "DeviceImage.h"
#include "vkRendererUtil.hpp"

#include "ShaderModuleInfo.hpp"

namespace LT {
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