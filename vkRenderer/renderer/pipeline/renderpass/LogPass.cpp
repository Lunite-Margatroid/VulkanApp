#include "vkRendererCommon.h"
#include "vkContext.h"
#include "LogPass.hpp"

#include "BufferManager.h"

#include "ImageManager.h"
#include "TextTable.hpp"

namespace LT {
	LogPass::LogPass()
	{
		vk::Device& device = vkContext::GetVkDevice();

		SetPrimitiveTopology(m_nFlag, vk::PrimitiveTopology::eTriangleList);
		SetPolygonMode(m_nFlag, vk::PolygonMode::eFill);
		SetBackCull(m_nFlag, false);
		SetBlendEnable(m_nFlag, true);
		// UV0 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 0);
		// UV1 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 1);

		// Init Pipeline
		std::vector<vk::DescriptorSetLayout> layouts;

		vk::DescriptorSetLayoutBinding binding = {};
		binding
			.setBinding(0)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			;

		vk::DescriptorSetLayoutCreateInfo dslci = {};
		dslci
			.setPBindings(&binding)
			.setBindingCount(1)
			;
		layouts.push_back(device.createDescriptorSetLayout(dslci));

		std::vector<vk::PushConstantRange> pushConstantRanges;
		vk::PushConstantRange pcr;
		pcr
			.setOffset(0)
			.setSize(sizeof(float) * 2)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);
		pushConstantRanges.push_back(pcr);
		Init(layouts, pushConstantRanges);
	}

	LogPass::~LogPass() {
	}
}