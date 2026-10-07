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
		m_mapSlots[BindingInfo(0u, BindingSpace::eVertexShader)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, -1);
		m_mapSlots[BindingInfo(1u, BindingSpace::eFragmentShader)] = MaterialSlot(vk::DescriptorType::eCombinedImageSampler, -1);

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

			pPass->AddShaderModule("MaterialTexampleProp", GenMtlPropShaderModule());
			pPass->AddShaderModule("FragmentShaderExample");
			pPass->AddShaderModule("CommonVertexShader");

			std::vector<vk::DescriptorSetLayout> vecSetLayout;

			auto arrMtlPropDescSet = GetMtlPropDescriptorSetLayout();
			vecSetLayout.insert(vecSetLayout.end(), arrMtlPropDescSet.begin(), arrMtlPropDescSet.end());
			auto vkTransBufferSetLayout = ShaderResourceManager::GetTransBufferDescriptorSetLayout();
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

	void BaseMaterial<MaterialExample, MaterialType::eExample>::RegisterMaterialProp() {

		for (int i = 0; i < static_cast<int>(MtlProp::MtlPropCount); ++i) {
			AddMaterialProp(static_cast<MtlProp>(i));
		}
	}
} // namespace