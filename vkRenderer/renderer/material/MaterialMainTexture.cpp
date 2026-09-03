// 测试材质 单一贴图 无光照
#include "vkRendererCommon.h"
#include "MaterialMainTexture.hpp"
#include "GraphicPass.hpp"

namespace LT {
	MaterialMainTexture::MaterialMainTexture(MaterialID nID)
		:IMaterial(nID)
	{
		m_vecSlots.push_back(MaterialSlot({ {0u, BindingSpace::eVertexShader} , vk::DescriptorType::eUniformBuffer, -1 }));
		m_vecSlots.push_back(MaterialSlot({ {1u, BindingSpace::eFragmentShader} , vk::DescriptorType::eCombinedImageSampler, -1 }));

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
} // namespace