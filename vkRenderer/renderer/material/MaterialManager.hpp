// 材质 MaterialManager

#pragma once

#include "vkRendererUtil.hpp"
#include "IMaterial.hpp"

namespace LT {
	class MaterialManager {
	private:
		using MaterialPtr = util::PtrWithRefCount<IMaterial>;

	private:
		std::map<MaterialID, MaterialPtr> m_mapMaterial;
		uint64_t m_nIDCounter;

	private:
		MaterialManager();
		~MaterialManager();

	public:
		unsigned int RefIncrease(MaterialID id;);
		unsigned int RefDecrease();

	public:
		static void Init();
		static void Release();
		static MaterialManager& GetInstance();
	private:
		static MaterialManager* s_pInstance;
	};
}