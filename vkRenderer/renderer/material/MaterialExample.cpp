// 示例材质：演示材质属性布局（Float3 + Image + Float + Float）
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "MaterialExample.hpp"
#include "GraphicPass.hpp"
#include "ImageManager.h"
#include "SamplerManager.h"

namespace LT {

	MaterialExample::MaterialExample(MaterialID nID)
		:BaseMaterial<MaterialExample, MaterialType::eExample>(nID)
	{
		RegisterStage(RenderStageType::eOpaqueForward);
	}

	RenderPass* MaterialExample::Bind(const MaterialBindInfo& sMtlBindInfo)
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

			pPass->AddShaderModule("MaterialTexampleProp", GenMtlPropShaderModule(), std::vector<std::string>());
			pPass->AddShaderModule("FragmentShaderExample", { "MaterialTexampleProp" });
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
		pPass->SetDescriptorSets({ ShaderResourceManager::GetTransBufferDescriptorSet(sMtlBindInfo.nFlightIndex), s_sDescriptorSets.GetDescriptorSet(sMtlBindInfo.nFlightIndex)});
		return pPass;
	}

	void BaseMaterial<MaterialExample, MaterialType::eExample>::RegisterMaterialProp() {

		for (int i = 0; i < static_cast<int>(MtlProp::MtlPropCount); ++i) {
			AddMaterialProp(static_cast<MtlProp>(i));
		}
	}
} // namespace