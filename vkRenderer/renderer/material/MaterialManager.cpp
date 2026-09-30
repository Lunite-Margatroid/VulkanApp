// 材质 MaterialManager
#include "vkRendererCommon.h"


#define SINGLETON_MANAGER_CUSTOMED_INIT_AND_RELEASE

#include "MaterialManager.hpp"
#include "MaterialMainTexture.hpp"
#include "MaterialExample.hpp"

namespace LT {

	template<typename ...Args>
	struct _MaterialRegistry {
		template<typename Func>
		static void ForEach(Func && func)
		{
			(func.template operator() <Args> (), ...);
		}
	};
	// 注册材质
	using MaterialRegistry = _MaterialRegistry<MaterialMainTexture, MaterialExample>;


	IMPLEMENT_SINGLETON_MANAGER(MaterialManager, IMaterial, MaterialID, Material)

	// 初始化
	IMPLEMENT_SINGLETON_MANAGER_INIT_BEGIN(MaterialManager)
	{
		MaterialRegistry::ForEach(
			[]<typename M>() {
			M::RegisterMaterialProp();
		}
		
		);

		MaterialRegistry::ForEach(
			[]<typename M>() {
			M::InitMaterialPropDataLayout();
		}

		);

		std::map<vk::DescriptorType, uint32_t> mapDescriptorCount;

		// Trans Buffer
		mapDescriptorCount[vk::DescriptorType::eUniformBuffer] += 1;
		// Get From Material
		MaterialRegistry::ForEach(
			[&]<typename M>() {
			M::GetDescriptorCount(mapDescriptorCount);
		}
		);

		ShaderResourceManager::CreateDescriptorPool(mapDescriptorCount);
	}
	IMPLEMENT_SINGLETON_MANAGER_INIT_END(MaterialManager)

	// 释放
	IMPLEMENT_SINGLETON_MANAGER_RELEASE_BEGIN(MaterialManager)
	{
		// 遍历已注册材质
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

} // namespace