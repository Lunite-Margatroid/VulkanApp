// 材质 MaterialManager
#include "vkRendererCommon.h"


#define SINGLETON_MANAGER_CUSTOMED_INIT_AND_RELEASE

#include "MaterialManager.hpp"
#include "MaterialMainTexture.hpp"

namespace LT {

	template<typename ...Args>
	struct _MaterialRegistry {
		template<typename Func>
		static void ForEach(Func&& func)
		{
			(func.template operator() <Args> (), ...);
		}
	};
	// 注册材质
	using MaterialRegistry = _MaterialRegistry<MaterialMainTexture>;


	IMPLEMENT_SINGLETON_MANAGER(MaterialManager, IMaterial, MaterialID, Material)


	IMPLEMENT_SINGLETON_MANAGER_INIT_BEGIN(MaterialManager)
	{

	}
	IMPLEMENT_SINGLETON_MANAGER_INIT_END(MaterialManager)


	IMPLEMENT_SINGLETON_MANAGER_RELEASE_BEGIN(MaterialManager)
	{
		MaterialRegistry::ForEach(
			[]<typename M>() {
			MaterialManager::UnregisterMaterial<M>();
			});
	}
	IMPLEMENT_SINGLETON_MANAGER_RELEASE_END(MaterialManager)


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

	void RegisterMaterial(MaterialType eType)
	{
	}

} // namespace