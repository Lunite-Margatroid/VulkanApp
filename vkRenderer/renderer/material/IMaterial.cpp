// 材质基类
#include "vkRendererCommon.h"
#include "IMaterial.hpp"

namespace LT {

	MtlPropDescriptorSets::MtlPropDescriptorSets()
	{
		for (_DescSets& sets : m_descriptorSets)
		{
			sets.fill(VK_NULL_HANDLE);
		}
	}

	// 如果不存在任何有效的DescriptorSet 返回false
	MtlPropDescriptorSets::operator bool() const {
		for (const _DescSets& sets : m_descriptorSets)
		{
			for (const vk::DescriptorSet& descSet : sets)
			{
				if (descSet)
				{
					return true;
				}
			}
		}
		return false;
	}

	void MtlPropDescriptorSets::Init(const std::array<vk::DescriptorSetLayout, static_cast<size_t>(BindingSpace::BindingSpaceCount)>& layouts) {
		vk::Device& device = vkContext::GetVkDevice();

		vk::DescriptorSetAllocateInfo dsai;
		dsai
			.setDescriptorPool(ShaderResourceManager::GetMtlPropDescriptorPool())
			.setDescriptorSetCount(layouts.size())
			.setPSetLayouts(layouts.data())
			;

		for (int i = 0; i < m_descriptorSets.size(); ++i)
		{
			std::vector<vk::DescriptorSet> sets = device.allocateDescriptorSets(dsai);
			RENDERER_ASSERT(sets.size() == m_descriptorSets[i].size(), "out of bound.");
			for (int j = 0; j < m_descriptorSets[i].size(); ++j)
			{
				m_descriptorSets[i][j] = sets[j];
			}
		}

	}

	vk::DescriptorSet MtlPropDescriptorSets::GetDescriptorSet(FlightFrameIndex nFlightIndex, BindingSpace eSpace)
	{
		return m_descriptorSets[nFlightIndex][static_cast<size_t>(eSpace)];
	}

	std::array<vk::DescriptorSet, static_cast<size_t>(BindingSpace::BindingSpaceCount)> MtlPropDescriptorSets::GetDescriptorSet(FlightFrameIndex nFlightIndex)
	{
		return m_descriptorSets[nFlightIndex];
	}

	IMaterial::IMaterial(MaterialID nID, MaterialType eType)
		:m_nID(nID)
		, m_eMtlType(eType)
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