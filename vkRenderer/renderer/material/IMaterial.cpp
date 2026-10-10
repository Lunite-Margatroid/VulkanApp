// 材质基类
#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "IMaterial.hpp"

namespace LT {

	MtlPropDescriptorSets::MtlPropDescriptorSets()
	{
	}

	// 如果不存在任何有效的DescriptorSet 返回false
	MtlPropDescriptorSets::operator bool() const {

		for (const DescriptorSetWrapper& descSet : m_descriptorSets)
		{
			if (descSet)
			{
				return true;
			}
		}

		return false;
	}

	void MtlPropDescriptorSets::Init(const vk::DescriptorSetLayout& layout) {
		vk::Device& device = vkContext::GetVkDevice();

		vk::DescriptorSetAllocateInfo dsai;
		dsai
			.setDescriptorPool(ShaderResourceManager::GetMtlPropDescriptorPool())
			.setDescriptorSetCount(1)
			.setPSetLayouts(&layout)
			;

		for (int i = 0; i < m_descriptorSets.size(); ++i)
		{
			std::vector<vk::DescriptorSet> sets = device.allocateDescriptorSets(dsai);
			m_descriptorSets[i] = sets[0];
		}

	}

	void MtlPropDescriptorSets::Release() {
		vk::Device& device = vkContext::GetVkDevice();
		for (auto& descriptorSet : m_descriptorSets)
		{
			descriptorSet.Destroy(ShaderResourceManager::GetMtlPropDescriptorPool());
		}
	}

	vk::DescriptorSet MtlPropDescriptorSets::GetDescriptorSet(FlightFrameIndex nFlightIndex)
	{
		return m_descriptorSets[nFlightIndex].GetNativeDescriptorSet();
	}

	IMaterial::IMaterial(MaterialID nID, MaterialType eType)
		:m_nID(nID)
		, m_eMtlType(eType)
	{
		m_mapSlots[BindingInfo(MTL_TRANS_BUFFER_BINDING_INDEX, ShaderStage::eVertexShader, ShaderResSpace::eTransBuffer)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, -1);
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
		m_mapSlots[BindingInfo(MTL_TRANS_BUFFER_BINDING_INDEX, ShaderStage::eVertexShader, ShaderResSpace::eTransBuffer)] = MaterialSlot(vk::DescriptorType::eUniformBuffer, nHandle);
		return EngineResult::eSuccess;
	}
} // namespace