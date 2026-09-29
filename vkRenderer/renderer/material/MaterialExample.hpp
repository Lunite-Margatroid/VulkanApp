#pragma once
#include "IMaterial.hpp"

namespace LT {
	class MaterialExample final : public BaseMaterial<MaterialExample, MaterialType::eExample> {
		friend class MaterialManager;

	protected:
		MaterialExample(MaterialID nID);
		~MaterialExample() = default;

		// 禁止拷贝和移动
		MaterialExample(const MaterialExample&) = delete;
		MaterialExample(MaterialExample&&) = delete;
		MaterialExample& operator = (const MaterialExample&) = delete;
		MaterialExample& operator = (MaterialExample&&) = delete;

	public:
		RenderPass* GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag) override;
		void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag, FlightFrameIndex nFlightFrameIndex) override;
	};

} // namespace LT