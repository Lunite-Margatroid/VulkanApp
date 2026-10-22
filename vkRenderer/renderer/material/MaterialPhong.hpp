// Phong材质 经典Phong光照
#pragma once
#include "BaseMaterial.hpp"

namespace LT {
	class MaterialPhong final : public BaseMaterial<MaterialPhong, MaterialType::ePhong> {
		friend class MaterialManager;

	protected:
		MaterialPhong(MaterialID nID);
		~MaterialPhong() = default;

		// 禁止拷贝和移动
		MaterialPhong(const MaterialPhong&) = delete;
		MaterialPhong(MaterialPhong&&) = delete;
		MaterialPhong& operator = (const MaterialPhong&) = delete;
		MaterialPhong& operator = (MaterialPhong&&) = delete;
	};

	using MaterialPhongBase = BaseMaterial<MaterialPhong, MaterialType::ePhong>;
} // namespace LT
