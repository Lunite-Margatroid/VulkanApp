#pragma once
#include "Buffer.h"
#include "DescriptorSetWrapper.hpp"

namespace LT {
	constexpr uint32_t LIGHT_UNIFORM_BUFFER_BINDING_INDEX = 0;
	constexpr uint32_t LIGHT_UNIFORM_BUFFER_DESC_SET_INDEX = 2;
	constexpr vk::ShaderStageFlags LIGHT_UNIFORM_BUFFER_STAGE_FLAG = vk::ShaderStageFlagBits::eFragment;

	// 不是一个单例
	// ShaderResourceManager和Pipeline使用
	class LightManager
	{
		friend class ShaderResourceManager;
	private:
		ConstBufferHandle m_nLightBuffer;

		std::array<uint32_t, LIGHT_GROUP_MAX_COUNT> m_arrLightCount;
		vk::DescriptorPool m_vkDescriptorPool;
		DescriptorSetWrapper m_sDescritporSet;

	public:
		void ClearLightBuffer();

		void PushLight(const std::bitset<LIGHT_GROUP_MAX_COUNT>& bitLightGroupFlag, const LightUniformBuffer& sLightBuffer);

		void UpdateBufferToDevice();

		DescriptorSetWrapper& GetDescriptorSetWrapper();

		LightManager(vk::DescriptorPool vkDescriptorPool);

		ConstBufferHandle GetBufferHandle() const;

		~LightManager();
	};
} // namespace