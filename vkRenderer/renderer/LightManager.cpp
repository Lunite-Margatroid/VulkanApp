#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "BufferManager.h"
#include "LightManager.hpp"
#include "vkContext.h"

#include "ShaderResourceManager.hpp"

namespace LT {

	LightManager::LightManager(vk::DescriptorPool vkDescriptorPool)
		:m_vkDescriptorPool(vkDescriptorPool)
	{
		m_nLightBuffer = ShaderResourceManager::CreateLightBufferHandle();

		m_sDescritporSet.Init(vkDescriptorPool, ShaderResourceManager::GetLightBufferDescriptorSetLayout());

		ClearLightBuffer();
	}
	ConstBufferHandle LightManager::GetBufferHandle() const
	{
		return m_nLightBuffer;
	}
	void LightManager::ClearLightBuffer()
	{
		BufferID nBufferID = ShaderResourceManager::GetConstBufferBinding(m_nLightBuffer).nBufferID;
		ConstBuffer* pConstBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(nBufferID));

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
		BufferID nBufferID = ShaderResourceManager::GetConstBufferBinding(m_nLightBuffer).nBufferID;

		ConstBuffer* pConstBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(nBufferID));
		LightUniformBuffer* pLightBuffer = reinterpret_cast<LightUniformBuffer*>(pConstBuffer->Data());
		for (uint32_t i = 0; i < LIGHT_GROUP_MAX_COUNT; i++)
		{
			if (bitLightGroupFlag.test(i) && m_arrLightCount[i] < LIGHT_MAX_COUNT_PER_GROUP_FORWARD) {
				pLightBuffer[i * LIGHT_MAX_COUNT_PER_GROUP_FORWARD + m_arrLightCount[i]] = sLightBuffer;
				m_arrLightCount[i] += 1;
			}
		}
	}

	void LightManager::UpdateBufferToDevice()
	{
		BufferID nBufferID = ShaderResourceManager::GetConstBufferBinding(m_nLightBuffer).nBufferID;
		ConstBuffer* pConstBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(nBufferID));
		pConstBuffer->UpdateConstBuffer();
	}

	DescriptorSetWrapper& LightManager::GetDescriptorSetWrapper()
	{
		return m_sDescritporSet;
	}

	LightManager::~LightManager()
	{
		ShaderResourceManager::ReleaseLightBufferHandle(m_nLightBuffer);
		m_sDescritporSet.Destroy(m_vkDescriptorPool);
	}
} // namespace