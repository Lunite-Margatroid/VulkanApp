// 材质 MaterialManager

#pragma once

#include "IMaterial.hpp"
#include "ManagerTemplate.hpp"

namespace LT {

	DECLEAR_SINGLETON_MANAGER_BEGIN(MaterialManager, IMaterial, MaterialID, Material)


public:
	static _MaterialRef CreateMaterial(MaterialType eType);

private:

	_MaterialRef m_refDefaultErrorMtl;

public:
	_MaterialRef GetDefaultErrorMaterial();

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

	template<typename TMaterial>
	static _MaterialRef CreateMaterial() {

		MaterialManager& mgr = GetInstance();
		MaterialID nID = mgr.GenID();
		TMaterial* pMtl = new TMaterial(nID);
		if (pMtl)
		{
			return mgr.Insert(pMtl->GetMaterialID(), pMtl);
		}

		return _MaterialRef(INVALID_MATERIAL_ID);
	}

	DECLEAR_SINGLETON_MANAGER_END(MaterialManager, IMaterial, MaterialID, Material)
}