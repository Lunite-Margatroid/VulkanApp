// 测试材质 单一贴图 无光照
#include "vkRendererCommon.h"
#include "MaterialMainTexture.hpp"
#include "GraphicPass.hpp"
#include "ImageManager.h"
#include "SamplerManager.h"

namespace LT {

	MaterialMainTexture::MaterialMainTexture(MaterialID nID)
		:BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture>(nID)
	{
		m_mapSlots[BindingInfo(1u, BindingSpace::eFragmentShader)] = MaterialSlot(vk::DescriptorType::eCombinedImageSampler, -1);

		RegisterStage(RenderStageType::eOpaqueForward);
	}
	RenderPass* MaterialMainTexture::Bind(const MaterialBindInfo& sMtlBindInfo)
	{
		GraphicPass* pPass = nullptr;

		auto iterPasses = s_mapRenderPasses.find(sMtlBindInfo.eStage);
		if (iterPasses == s_mapRenderPasses.end())
		{
			return nullptr;
		}
		auto& mapPass = iterPasses->second;

		auto iterPass = mapPass.find(sMtlBindInfo.nFlag);
		if (iterPass == mapPass.end())
		{
			pPass = new GraphicPass();
			pPass->SetRenderPassFlag(sMtlBindInfo.nFlag);
			pPass->AddShaderModule("MaterialMainTextureProp", GenMtlPropShaderModule());
			pPass->AddShaderModule("FragmentShaderMainTex");
			pPass->AddShaderModule("CommonVertexShader");

			std::vector<vk::DescriptorSetLayout> vecSetLayout;

			auto arrMtlPropDescSet = GetMtlPropDescriptorSetLayout();
			vecSetLayout.insert(vecSetLayout.end(), arrMtlPropDescSet.begin(), arrMtlPropDescSet.end());
			auto vkTransBufferSetLayout = ShaderResourceManager::GetTransBufferDescLayout();
			vecSetLayout.push_back(vkTransBufferSetLayout);

			pPass->Init(vecSetLayout);

			mapPass[sMtlBindInfo.nFlag] = pPass;
		}
		else
		{
			pPass = reinterpret_cast<GraphicPass*>(iterPass->second);
		}

		BindShaderResource(sMtlBindInfo);

		return pPass;
	}

	void MaterialMainTexture::SetMainTexture(ImageID nMainTex)
	{
		SetSlotSrc(BindingInfo(1u, BindingSpace::eFragmentShader), nMainTex);
	}

	void BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture>::RegisterMaterialProp() {
		AddMaterialProp(MtlProp::eTexDiffuse);
	}
} // namespace