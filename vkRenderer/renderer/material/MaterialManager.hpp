// 材质 MaterialManager

#pragma once

#include "IMaterial.hpp"
#include "ManagerTemplate.hpp"

namespace LT {

	DECLEAR_SINGLETON_MANAGER_BEGIN(MaterialManager, IMaterial, MaterialID, Material)

public:
	static _MaterialRef CreateMaterial(MaterialType eType);

	DECLEAR_SINGLETON_MANAGER_END(MaterialManager, IMaterial, MaterialID, Material)
}