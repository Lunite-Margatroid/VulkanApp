// 测试材质 单一贴图 无光照
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "MaterialMainTexture.hpp"
#include "GraphicPass.hpp"
#include "ImageManager.h"
#include "SamplerManager.h"

namespace LT {

	MaterialMainTexture::MaterialMainTexture(MaterialID nID)
		:BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture>(nID)
	{
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
			pPass->AddShaderModule("MaterialMainTextureProp", GenMtlPropShaderModule(), std::vector<std::string>());
			pPass->AddShaderModule("FragmentShaderMainTex", { "MaterialMainTextureProp" });
			pPass->AddShaderModule("CommonVertexShader", std::vector<std::string>());

			std::vector<vk::DescriptorSetLayout> vecSetLayout;


			//跟 enum ShaderResSpace 的顺序保持一致
			//	eTransBuffer = 0,
			//	eMtlProp = 1,

			// 0 Trans Buffer
			auto vkTransBufferSetLayout = ShaderResourceManager::GetTransBufferDescriptorSetLayout();
			vecSetLayout.push_back(vkTransBufferSetLayout);
			// 1 Mtl Prop
			auto vkMtlPropDescSet = GetMtlPropDescriptorSetLayout();
			vecSetLayout.push_back(vkMtlPropDescSet);

			pPass->Init(vecSetLayout);

			mapPass[sMtlBindInfo.nFlag] = pPass;
		}
		else
		{
			pPass = reinterpret_cast<GraphicPass*>(iterPass->second);
		}

		BindShaderResource(sMtlBindInfo);
		pPass->SetDescriptorSets({ ShaderResourceManager::GetTransBufferDescriptorSet(sMtlBindInfo.nFlightIndex), s_sDescriptorSets.GetDescriptorSet(sMtlBindInfo.nFlightIndex) });
		return pPass;
	}

	void BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture>::RegisterMaterialProp() {
		AddMaterialProp(MtlProp::eTexDiffuse);
	}
} // namespace