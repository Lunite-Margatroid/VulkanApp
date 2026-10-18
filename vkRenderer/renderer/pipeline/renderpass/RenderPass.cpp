#include "vkRendererCommon.h"
#include "RenderPass.hpp"
#include "ImageManager.h"

namespace LT {

	std::array<uint32_t, static_cast<size_t>(MtlProp::MtlPropCount)> g_arrMtlTex2TexIndex = [] {
		uint32_t nIndex = 0;
		std::array<uint32_t, static_cast<size_t>(MtlProp::MtlPropCount)> result;
		for (int i = 0; i < result.size(); ++i)
		{
			if (GetMtlPropDataType(static_cast<MtlProp>(i)) == MtlPropDataType::eImage) {
				result[i] = nIndex++;
			}
			else
			{
				result[i] = std::numeric_limits<uint32_t>::max();
			}
		}
		RENDERER_ASSERT(nIndex > MTL_PROP_TEX_MAX_COUNT, "Tex Prop is Too Many.");
		return result;
		}();

	std::array<MtlProp, MTL_PROP_TEX_MAX_COUNT> g_arrMtlPropTex2MtlPropEnum = { MtlProp::eInvalid };

	// 记录每个纹理槽位的宏定义名称 "MTL_PROP_" + 属性名
	// 由 ToString(MtlProp eProp) 生成
	std::array<std::string, MTL_PROP_TEX_MAX_COUNT> g_arrMtlPropTexMacroName = [] {
		std::array<std::string, MTL_PROP_TEX_MAX_COUNT> arrName;
		for (uint32_t nTexSlotIndex = 0; nTexSlotIndex < MTL_PROP_TEX_MAX_COUNT; ++nTexSlotIndex)
		{
			MtlProp eProp = MtlPropTexIndexToMtlPropEnum(nTexSlotIndex);
			if (eProp != MtlProp::eInvalid)
			{
				arrName[nTexSlotIndex] = "MTL_PROP_" + std::string(ToString(eProp));
			}
		}
		return arrName;
	}();


	uint32_t MtlPropEnum2TexIndex(MtlProp eProp) {
		return g_arrMtlTex2TexIndex[static_cast<uint32_t>(eProp)];
	}

	MtlProp MtlPropTexIndexToMtlPropEnum(int32_t nIndex) {
		if (g_arrMtlPropTex2MtlPropEnum[0] == MtlProp::eInvalid)
		{
			for (int i = 0; i < static_cast<int>(MtlProp::MtlPropCount); ++i)
			{
				uint32_t tIndex = MtlPropEnum2TexIndex(static_cast<MtlProp>(i));
				if (tIndex < MTL_PROP_TEX_MAX_COUNT)
				{
					g_arrMtlPropTex2MtlPropEnum[tIndex] = static_cast<MtlProp>(i);
				}
				else
				{
					g_arrMtlPropTex2MtlPropEnum[tIndex] = MtlProp::eInvalid;
				}
			}
		}
		if (nIndex >= g_arrMtlPropTex2MtlPropEnum.size())
			return MtlProp::eInvalid;
		return g_arrMtlPropTex2MtlPropEnum[nIndex];
	}

    std::vector<std::pair<const char*, const char*>> GenGraphicPPMacroDesc(const RenderPassFlag& nFlag)
    {
		std::vector<std::pair<const char*, const char*>> vecPPMacro;
		if (HasVertexPos(nFlag)) {
			vecPPMacro.emplace_back("VERT_POSITION", "1");
		}
		if (HasUV(nFlag, 0))
		{
			vecPPMacro.emplace_back("VERT_UV0", "1");
		}
		if (HasUV(nFlag, 1))
		{
			vecPPMacro.emplace_back("VERT_UV1", "1");
		}
		if (HasUV(nFlag, 2))
		{
			vecPPMacro.emplace_back("VERT_UV2", "1");
		}
		if (HasUV(nFlag, 3))
		{
			vecPPMacro.emplace_back("VERT_UV3", "1");
		}
		if (HasUV(nFlag, 4))
		{
			vecPPMacro.emplace_back("VERT_UV4", "1");
		}
		if (HasNormal(nFlag))
		{
			vecPPMacro.emplace_back("VERT_NORMAL", "1");
		}
		if (HasTangent(nFlag))
		{
			vecPPMacro.emplace_back("VERT_TANGENT", "1");
		}
		if (HasBitangent(nFlag))
		{
			vecPPMacro.emplace_back("VERT_BITANGENT", "1");
		}
		if (HasColor(nFlag))
		{
			vecPPMacro.emplace_back("VERT_COLOR", "1");
		}
		if (HasAO(nFlag))
		{
			vecPPMacro.emplace_back("VERT_AO", "1");
		}
		// 材质属性贴图
		for (uint32_t nTexSlotIndex = 0; nTexSlotIndex < MTL_PROP_TEX_MAX_COUNT; ++nTexSlotIndex)
		{
			if (HasMtlPropTex(nFlag, nTexSlotIndex) && !g_arrMtlPropTexMacroName[nTexSlotIndex].empty())
			{
				vecPPMacro.emplace_back(g_arrMtlPropTexMacroName[nTexSlotIndex].c_str(), "1");
			}
		}
		return vecPPMacro;
    }


	void RenderPass::RecordTransitionImageLayout(const TransitionImageLayoutInfo& sInfo) {
		vk::ImageMemoryBarrier2 barrier = {};
		barrier
			.setImage(ImageManager::GetNativeDeviceImage(sInfo.nImageID))
			.setSrcAccessMask(sInfo.srcAccessFlag)
			.setSrcStageMask(sInfo.srcStageMask)
			.setOldLayout(sInfo.eOldLayout)
			.setDstAccessMask(sInfo.dstAccessFlag)
			.setDstStageMask(sInfo.dstStageMask)
			.setNewLayout(sInfo.eNewLayout)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(
				vk::ImageSubresourceRange(
					sInfo.eImageAspect,
					0,	// base mipmap level
					1,	// level count
					0,	// base array layer
					1	// layer count
				)
			)
			;

		vk::DependencyInfo di = {};
		di
			.setDependencyFlags({})
			.setImageMemoryBarrierCount(1)
			.setPImageMemoryBarriers(&barrier)
			;

		sInfo.pCmdBuffer->PipelineBarrier2(di);
	}
} // namespace LT
