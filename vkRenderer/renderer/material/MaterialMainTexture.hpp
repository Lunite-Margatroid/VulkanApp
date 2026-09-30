// 测试材质 单一贴图 无光照
#pragma once
#include "IMaterial.hpp"


namespace LT {
	class MaterialMainTexture final : public BaseMaterial<MaterialMainTexture, MaterialType::eMainTexture> {
		friend class MaterialManager;

	protected:
		MaterialMainTexture(MaterialID nID);
		~MaterialMainTexture() = default;

		MaterialMainTexture& operator = (const MaterialMainTexture&) = delete;
		MaterialMainTexture& operator = (MaterialMainTexture&&) = delete;
		MaterialMainTexture(const MaterialMainTexture&) = delete;
		MaterialMainTexture(MaterialMainTexture&&) = delete;

	public:
		RenderPass* GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag) override;

		void SetMainTexture(ImageID nMainTex);
	};

} // namespace LT