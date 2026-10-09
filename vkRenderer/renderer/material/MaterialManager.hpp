// 材质 MaterialManager

#pragma once

#include "IMaterial.hpp"
#include "ManagerTemplate.hpp"

namespace LT {

	DECLEAR_SINGLETON_MANAGER_BEGIN(MaterialManager, IMaterial, MaterialID, Material)
public:
	static _MaterialRef CreateMaterial(MaterialType eType);

private:
	template<typename TMaterial>
	static void UnregisterMaterial() {
		for (auto& mapPasses : TMaterial::s_mapRenderPasses)
		{
			for (auto& pass : mapPasses.second) {
				delete pass.second;
			}
			mapPasses.second.clear();
		}
		TMaterial::s_mapRenderPasses.clear();

		TMaterial::UnresigterMaterial();
	}

	DECLEAR_SINGLETON_MANAGER_END(MaterialManager, IMaterial, MaterialID, Material)
}