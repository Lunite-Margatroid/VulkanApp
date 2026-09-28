// 材质基类
#include "vkRendererCommon.h"
#include "IMaterial.hpp"

namespace LT {
	IMaterial::IMaterial(MaterialID nID) 
		:m_nID(nID)
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