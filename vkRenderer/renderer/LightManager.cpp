#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "BufferManager.h"
#include "LightManager.hpp"
#include "vkContext.h"

namespace LT {

	LightManager::LightManager(vk::DescriptorPool vkDescriptorPool)
		:m_vkDescriptorPool(vkDescriptorPool)
	{
		m_nLightBuffer = BufferManager::CreateConstBuffer(sizeof(LightUniformBuffer) * LIGHT_GROUP_MAX_COUNT * LIGHT_MAX_COUNT_PER_GROUP_FORWARD, nullptr)->GetBufferID();

		vk::DescriptorSetLayoutBinding dslb = {};
		dslb
			.setBinding(LIGHT_UNIFORM_BUFFER_BINDING_INDEX)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(LIGHT_UNIFORM_BUFFER_STAGE_FLAG)
			;

		vk::DescriptorSetLayoutCreateInfo dslci = {};
		dslci
			.setBindingCount(1)
			.setPBindings(&dslb)
			;

		m_vkLayout = vkContext::GetVkDevice().createDescriptorSetLayout(dslci);

		m_sDescritporSet.Init(vkDescriptorPool, m_vkLayout);
	}
	void LightManager::ClearLightBuffer()
	{
		ConstBuffer* pConstBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(m_nLightBuffer));
		LightUniformBuffer* pLightBuffer = reinterpret_cast<LightUniformBuffer*>(pConstBuffer->Data());
		if (pLightBuffer)
		{
			for (int i = 0; i < LIGHT_GROUP_MAX_COUNT * LIGHT_MAX_COUNT_PER_GROUP_FORWARD; i++)
			{
				pLightBuffer[i].nLightType = LightType::eNone;
			}
		}
		pConstBuffer->UpdateConstBuffer();

		m_arrLightCount.fill(0);
	}

	void LightManager::PushLight(const std::bitset<LIGHT_GROUP_MAX_COUNT>& bitLightGroupFlag, const LightUniformBuffer& sLightBuffer)
	{
		ConstBuffer* pConstBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(m_nLightBuffer));
		LightUniformBuffer* pLightBuffer = reinterpret_cast<LightUniformBuffer*>(pConstBuffer->Data());
		for (uint32_t i = 0; i < LIGHT_GROUP_MAX_COUNT; i++)
		{
			if (bitLightGroupFlag.test(i)) {
				pLightBuffer[i * LIGHT_MAX_COUNT_PER_GROUP_FORWARD + m_arrLightCount[i]] = sLightBuffer;
				m_arrLightCount[i] += 1;
			}
		}
	}

	LightManager::~LightManager()
	{
		BufferManager::DeleteBuffer(m_nLightBuffer);
		if (m_vkLayout)
		{
			vkContext::GetVkDevice().destroyDescriptorSetLayout(m_vkLayout);
		}
		m_sDescritporSet.Destroy(m_vkDescriptorPool);
	}
} // namespace