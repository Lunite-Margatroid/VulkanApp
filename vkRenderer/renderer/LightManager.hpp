#pragma once
#include "Buffer.h"
#include "DescriptorSetWrapper.hpp"

namespace LT {
	constexpr uint32_t LIGHT_UNIFORM_BUFFER_BINDING_INDEX = 0;
	constexpr uint32_t LIGHT_UNIFORM_BUFFER_DESC_SET_INDEX = 2;
	constexpr vk::ShaderStageFlags LIGHT_UNIFORM_BUFFER_STAGE_FLAG = vk::ShaderStageFlagBits::eVertex;

	// 不是一个单例
	// ShaderResourceManager会使用该类
	class LightManager
	{
		friend class ShaderResourceManager;
	private:
		BufferID m_nLightBuffer;

		std::array<uint32_t, LIGHT_GROUP_MAX_COUNT> m_arrLightCount;
		vk::DescriptorPool m_vkDescriptorPool;
		// 储存灯光的Uniform Buffer的Layout
		vk::DescriptorSetLayout m_vkLayout;
		DescriptorSetWrapper m_sDescritporSet;

		void ClearLightBuffer();

		void PushLight(const std::bitset<LIGHT_GROUP_MAX_COUNT>& bitLightGroupFlag, const LightUniformBuffer& sLightBuffer);

		LightManager(vk::DescriptorPool vkDescriptorPool);
		~LightManager();

	public:
		
	};
} // namespace