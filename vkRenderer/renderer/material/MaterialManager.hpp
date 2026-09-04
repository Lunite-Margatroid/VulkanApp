// 材质 MaterialManager

#pragma once

#include "vkRendererUtil.hpp"
#include "IMaterial.hpp"

namespace LT {

	DECLEAR_SINGLETON_MANAGER_BEGIN(MaterialManager, IMaterial, MaterialID, Material)

public:
	static MaterialRef CreateMaterial(MaterialType eType);

	DECLEAR_SINGLETON_MANAGER_END(MaterialManager, IMaterial, MaterialID, Material)
}