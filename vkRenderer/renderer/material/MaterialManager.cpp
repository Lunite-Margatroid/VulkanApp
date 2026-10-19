// 材质 MaterialManager
#include "vkRendererCommon.h"
#include "EngineCommon.h"
// 允许额外的初始化和释放
#define SINGLETON_MANAGER_CUSTOMED_INIT_AND_RELEASE

#include "MaterialManager.hpp"
#include "MaterialMainTexture.hpp"
#include "MaterialExample.hpp"
#include "MaterialError.hpp"

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
	using MaterialRegistry = _MaterialRegistry<MaterialMainTexture, MaterialExample, MaterialError>;



	IMPLEMENT_SINGLETON_MANAGER(MaterialManager, IMaterial, MaterialID, Material)

	// 初始化
	IMPLEMENT_SINGLETON_MANAGER_INIT_BEGIN(MaterialManager)
	{
		MaterialRegistry::ForEach(
			[]<typename M>() {
			M::RegisterMaterialProp();
			M::InitMaterialPropDataLayout();
		}
		
		);


		MaterialRegistry::ForEach(
			[]<typename M>() {
			ShaderResourceManager::RegisterMaterial<M>();
		}
		);

		


		std::vector<std::pair<vk::DescriptorType, BindingInfo>> vecDescriptors;

		// Get From Material
		// 收集注册材质的用于属性的Decriptor
		MaterialRegistry::ForEach(
			[&]<typename M>() {
			M::GetDescriptorBindings(vecDescriptors);
		}
		);

		std::map<vk::DescriptorType, uint32_t> mapDescriptorCount;
		for (const auto& [eType, sBinding] : vecDescriptors)
		{
			mapDescriptorCount[eType] += 1;
		}

		ShaderResourceManager::CreateDescriptorPool(mapDescriptorCount);

		// 为Material Prop 创建DescriptorSet
		MaterialRegistry::ForEach(
			[]<typename M>() {
			M::InitMtlPropDescriptorSet();
		}

		);
	}
	IMPLEMENT_SINGLETON_MANAGER_INIT_END(MaterialManager)

	// 释放
	IMPLEMENT_SINGLETON_MANAGER_RELEASE_BEGIN(MaterialManager)
	{
		// 删除默认的材质
		MaterialManager& mgr = MaterialManager::GetInstance();
		if (mgr.m_refDefaultErrorMtl)
		{
			mgr.m_refDefaultErrorMtl = MaterialRef(INVALID_MATERIAL_ID);
		}

		// 遍历已注册材质
		MaterialRegistry::ForEach(
			[]<typename M>() {
			MaterialManager::UnregisterMaterial<M>();
			});
	}
	IMPLEMENT_SINGLETON_MANAGER_RELEASE_END(MaterialManager)


	MaterialRef MaterialManager::CreateMaterial(MaterialType eType) {

		MaterialManager& mgr = GetInstance();
		MaterialRef refMtl(INVALID_MATERIAL_ID);
		MaterialRegistry::ForEach([&]<typename T>() {
			if (T::GetMaterialType() == eType) {
				refMtl = MaterialManager::CreateMaterial<T>();
			}
		});
		return refMtl;
	}

	MaterialRef MaterialManager::GetDefaultErrorMaterial()
	{
		if (!m_refDefaultErrorMtl)
		{
			m_refDefaultErrorMtl = CreateMaterial(MaterialType::eError);
		}
		return m_refDefaultErrorMtl;
	}

} // namespace