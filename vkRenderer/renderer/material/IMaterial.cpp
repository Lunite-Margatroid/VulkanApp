// 材质基类
#include "vkRendererCommon.h"
#include "IMaterial.hpp"

namespace LT {
	IMaterial::IMaterial(MaterialID nID, MaterialType eType) 
		:m_nID(nID)
		,m_eMtlType(eType)
	{
		m_mapSlots[BindingInfo(MTL_TRANS_BUFFER_BINDING_INDEX, BindingSpace::eVertexShader)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, -1);
	}

	IMaterial::~IMaterial()
	{}

	EngineResult IMaterial::SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID)
	{
		auto iter = m_mapSlots.find(sBindingInfo);
		if (iter != m_mapSlots.end())
		{
			iter->second.nSrcID = nSrcID;
			return EngineResult::eSuccess;
		}
		return EngineResult::eFailed;
	}

	EngineResult IMaterial::SetTransBuffer(ConstBufferHandle nHandle)
	{
		m_mapSlots[BindingInfo(MTL_TRANS_BUFFER_BINDING_INDEX, BindingSpace::eVertexShader)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, nHandle);
		return EngineResult::eSuccess;
	}
} // namespace