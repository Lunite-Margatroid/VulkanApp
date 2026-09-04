// 材质 MaterialManager
#include "vkRendererCommon.h"
#include "MaterialManager.hpp"
#include "MaterialMainTexture.hpp"

namespace LT {
	IMPLEMENT_SINGLETON_MANAGER(MaterialManager, IMaterial, MaterialID, Material)

	MaterialRef MaterialManager::CreateMaterial(MaterialType eType) {

		MaterialManager& mgr = GetInstance();

		IMaterial* pMtl = nullptr;
		MaterialID nID = INVALID_MATERIAL_ID;
		switch (eType) {
			case MaterialType::eMainTexture:
				nID = mgr.GenID();
				pMtl = new MaterialMainTexture(nID);
				break;
			default:
				break;
		};

		return mgr.Insert(nID, pMtl);
	}

} // namespace