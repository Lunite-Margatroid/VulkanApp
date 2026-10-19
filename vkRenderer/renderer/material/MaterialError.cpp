// 错误材质 品红棋盘格 无材质属性
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "MaterialError.hpp"
#include "GraphicPass.hpp"
#include "ImageManager.h"
#include "SamplerManager.h"

namespace LT {

	void MaterialErrorBase::InitShaderModule(GraphicPass* pPass) {
		pPass->AddShaderModule("FragmentShaderError", std::vector<std::string>());
		pPass->AddShaderModule("CommonVertexShader", std::vector<std::string>());
	}

	MaterialError::MaterialError(MaterialID nID)
		:BaseMaterial<MaterialError, MaterialType::eError>(nID)
	{
		RegisterStage(RenderStageType::eOpaqueForward);
	}

	void BaseMaterial<MaterialError, MaterialType::eError>::RegisterMaterialProp() {
		// 错误材质没有材质属性
	}
} // namespace
