// Phong材质
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "MaterialPhong.hpp"
#include "GraphicPass.hpp"
#include "ImageManager.h"
#include "SamplerManager.h"

namespace LT {

	void MaterialPhongBase::InitShaderModule(GraphicPass* pPass) {
		pPass->AddShaderModule("MaterialPhongProp", GenMtlPropShaderModule(pPass->GetRenderPassFlag()), std::vector<std::string>());
		pPass->AddShaderModule("FragmentShaderPhong", { "MaterialPhongProp" });
		pPass->AddShaderModule("CommonVertexShader", std::vector<std::string>());
	}

	MaterialPhong::MaterialPhong(MaterialID nID)
		:BaseMaterial<MaterialPhong, MaterialType::ePhong>(nID)
	{
		RegisterStage(RenderStageType::eOpaqueForward);

		// 有灯光
		SetLighting(m_nMtlFlag, true);
	}

	void BaseMaterial<MaterialPhong, MaterialType::ePhong>::RegisterMaterialProp() {
		// 注册GenLocalSurface引用的属性
		AddMaterialProp(MtlProp::eAlpha);
		AddMaterialProp(MtlProp::eTexAlpha);
		AddMaterialProp(MtlProp::eColorDiffuse);
		AddMaterialProp(MtlProp::eTexDiffuse);
		AddMaterialProp(MtlProp::eColorSpecular);
		AddMaterialProp(MtlProp::eTexSpecular);
		AddMaterialProp(MtlProp::eMetalness);
		AddMaterialProp(MtlProp::eTexMeltalness);
		AddMaterialProp(MtlProp::eTexIOR);
		AddMaterialProp(MtlProp::eIOR);
	}
} // namespace
