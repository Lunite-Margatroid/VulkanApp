// 测试材质 单一贴图 无光照
#include "vkRendererCommon.h"
#include "MaterialMainTexture.hpp"
#include "GraphicPass.hpp"

namespace LT {

	MaterialMainTexture::MaterialMainTexture(MaterialID nID)
		:BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture>(nID)
	{
		m_mapSlots[BindingInfo(1u, BindingSpace::eFragmentShader)] = MaterialSlot(vk::DescriptorType::eCombinedImageSampler, -1);

		RegisterStage(RenderStageType::eOpaqueForward);
	}
	RenderPass* MaterialMainTexture::GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag)
	{
		auto iterPasses = s_mapRenderPasses.find(eStage);
		if (iterPasses == s_mapRenderPasses.end())
		{
			return nullptr;
		}
		auto& mapPass = iterPasses->second;

		auto iterPass = mapPass.find(nFlag);
		if (iterPass == mapPass.end())
		{
			GraphicPass* pPass = new GraphicPass();
			pPass->SetRenderPassFlag(nFlag);
			pPass->AddShaderModule("MaterialMainTextureProp", GenMtlPropShaderModule());
			pPass->AddShaderModule("FragmentShaderMainTex");
			pPass->AddShaderModule("CommonVertexShader");
			pPass->Init();

			mapPass[nFlag] = pPass;

			return pPass;
		}

		return iterPass->second;
	}

	void MaterialMainTexture::SetMainTexture(ImageID nMainTex)
	{
		SetSlotSrc(BindingInfo(1u, BindingSpace::eFragmentShader), nMainTex);
	}

	void BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture>::RegisterMaterialProp() {
		AddMaterialProp(MtlProp::eTexDiffuse);
	}
} // namespace