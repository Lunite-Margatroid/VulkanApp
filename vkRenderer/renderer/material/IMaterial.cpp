// 材质基类
#include "vkRendererCommon.h"
#include "IMaterial.hpp"

namespace LT {
	IMaterial::IMaterial(MaterialID nID, MaterialType eType) 
		:m_nID(nID)
		,m_eMtlType(eType)
	{}

	IMaterial::~IMaterial()
	{}

	ResultSetter IMaterial::SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID)
	{
		auto iter = m_mapSlots.find(sBindingInfo);
		if (iter != m_mapSlots.end())
		{
			iter->second.nSrcID = nSrcID;
			return 0;
		}
		return -1;
	}

} // namespace