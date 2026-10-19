// 错误材质 品红棋盘格 用于表示着色器编译错误或者材质ID不存在
#pragma once
#include "IMaterial.hpp"

namespace LT {
	class MaterialError final : public BaseMaterial<MaterialError, MaterialType::eError> {
		friend class MaterialManager;

	protected:
		MaterialError(MaterialID nID);
		~MaterialError() = default;

		MaterialError& operator = (const MaterialError&) = delete;
		MaterialError& operator = (MaterialError&&) = delete;
		MaterialError(const MaterialError&) = delete;
		MaterialError(MaterialError&&) = delete;

	public:
		//RenderPass* Bind(const MaterialBindInfo& sMtlBindInfo) override;
	};

	using MaterialErrorBase = BaseMaterial<MaterialError, MaterialType::eError>;
} // namespace LT
