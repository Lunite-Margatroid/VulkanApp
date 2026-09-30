// 示例材质：演示材质属性布局（Float3 + Image + Float + Float）
#include "vkRendererCommon.h"
#include "MaterialExample.hpp"
#include "GraphicPass.hpp"

namespace LT {

	MaterialExample::MaterialExample(MaterialID nID)
		:BaseMaterial<MaterialExample, MaterialType::eExample>(nID)
	{
		m_mapSlots[BindingInfo(0u, BindingSpace::eVertexShader)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, -1);
		m_mapSlots[BindingInfo(1u, BindingSpace::eFragmentShader)] = MaterialSlot(vk::DescriptorType::eCombinedImageSampler, -1);

		RegisterStage(RenderStageType::eOpaqueForward);
	}
	RenderPass* MaterialExample::GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag)
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

			pPass->AddShaderModule("MaterialTexampleProp", GenMtlPropShaderModule());
			pPass->AddShaderModule("FragmentShaderExample");
			pPass->AddShaderModule("CommonVertexShader");
			pPass->Init();

			mapPass[nFlag] = pPass;

			return pPass;
		}

		return iterPass->second;
	}

	void BaseMaterial<MaterialExample, MaterialType::eExample>::RegisterMaterialProp() {

		for (int i = 0; i < static_cast<int>(MtlProp::MtlPropCount); ++i) {
			AddMaterialProp(static_cast<MtlProp>(i));
		}
	}
} // namespace