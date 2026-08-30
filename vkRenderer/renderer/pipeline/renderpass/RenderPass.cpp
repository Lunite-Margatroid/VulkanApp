#include "vkRendererCommon.h"
#include "RenderPass.hpp"
#include "ImageManager.h"

namespace LT {

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

		sInfo.vkCommandBuffer.pipelineBarrier2(di);

	}
} // namespace LT
