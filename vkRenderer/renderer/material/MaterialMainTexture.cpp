// 测试材质 单一贴图 无光照
#include "vkRendererCommon.h"
#include "MaterialMainTexture.hpp"
#include "GraphicPass.hpp"

namespace LT {
	MaterialMainTexture::MaterialMainTexture(MaterialID nID)
		:IMaterial(nID)
	{
		m_mapSlots[BindingInfo(0u, BindingSpace::eVertexShader)] = MaterialSlot({ vk::DescriptorType::eUniformBuffer, -1 });
		m_mapSlots[BindingInfo(1u, BindingSpace::eFragmentShader)] = MaterialSlot({ vk::DescriptorType::eCombinedImageSampler, -1 });

		RegisterStage(RenderStageType::eOpaqueForward);
	}
	RenderPass* MaterialMainTexture::GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag)
	{
		auto iterPasses = m_mapRenderPasses.find(eStage);
		if (iterPasses == m_mapRenderPasses.end())
		{
			return nullptr;
		}
		RenderPassMap& mapPass = iterPasses->second;

		auto iterPass = mapPass.find(nFlag);
		if (iterPass == mapPass.end())
		{
			GraphicPass* pPass = new GraphicPass();
			pPass->SetRenderPassFlag(nFlag);
			pPass->AddShaderModule("FragmentShaderMainTex");
			pPass->AddShaderModule("CommonVertexShader");
			pPass->Init();

			mapPass[nFlag] = pPass;

			return pPass;
		}

		return iterPass->second;
	}
	void MaterialMainTexture::UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag, FlightFrameIndex nFlightFrameIndex)
	{
		GraphicPass* pRenderPass = dynamic_cast<GraphicPass*>(GetRenderPass(eStage, nFlag));
		if (pRenderPass) {
			for (const auto& [bindingInfo, mtlSlot] : m_mapSlots)
			{
				if (mtlSlot.nSrcID >= 0)
				{
					switch (mtlSlot.eDescType)
					{
						case vk::DescriptorType::eUniformBuffer:
							pRenderPass->BindConstBuffer(mtlSlot.nSrcID, bindingInfo.eSpace, bindingInfo.nIndex, nFlightFrameIndex);
							break;
						case vk::DescriptorType::eCombinedImageSampler:
							pRenderPass->BindImage2D(mtlSlot.nSrcID, bindingInfo.eSpace, bindingInfo.nIndex, nFlightFrameIndex);
							break;
						default:
							break;
					};
				}
			}
		}
	}

	void MaterialMainTexture::SetMainTexture(ImageID nMainTex)
	{
		SetSlotSrc(BindingInfo(1u, BindingSpace::eFragmentShader), nMainTex);
	}
} // namespace