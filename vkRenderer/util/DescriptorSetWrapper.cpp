#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DescriptorSetWrapper.hpp"
namespace LT {
	DescriptorSetWrapper::DescriptorSetWrapper() 
		: m_vkDescriptorSet(VK_NULL_HANDLE)
		, m_fenceDrawing(VK_NULL_HANDLE)
	{}

	DescriptorSetWrapper::DescriptorSetWrapper(vk::DescriptorSet vkDescriptorSet)
	{
		Init(vkDescriptorSet);
	}
	void DescriptorSetWrapper::Init(vk::DescriptorSet vkDescriptorSet) {
		m_vkDescriptorSet = vkDescriptorSet;
		m_fenceDrawing = vkContext::GetVkDevice().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	void DescriptorSetWrapper::Destroy(vk::DescriptorPool vkDescriptorPool)
	{
		vk::Device &device = vkContext::GetVkDevice();
		device.destroyFence(m_fenceDrawing);
		m_fenceDrawing = VK_NULL_HANDLE;
		std::array<vk::DescriptorSet, 1> arrDescSet{m_vkDescriptorSet};
		device.freeDescriptorSets(vkDescriptorPool, arrDescSet);
		m_vkDescriptorSet = VK_NULL_HANDLE;
	}

	vk::DescriptorSet DescriptorSetWrapper::GetNativeDescriptorSet()
	{
		vk::Device& device = vkContext::GetVkDevice();
		vk::Result result = device.waitForFences(m_fenceDrawing, vk::True, std::_Max_limit<uint64_t>());
		RENDERER_ASSERT(result == vk::Result::eSuccess, "FenceDrawing of Descriptor: Wait failed.");
		device.resetFences(m_fenceDrawing);

		return m_vkDescriptorSet;
	}
	vk::DescriptorSet DescriptorSetWrapper::GetNativeDescriptorSetWithoutWaiting()
	{
		return m_vkDescriptorSet;
	}
	vk::Fence DescriptorSetWrapper::GetFenceDrawing()
	{
		return m_fenceDrawing;
	}

	DescriptorSetWrapper::operator bool() const{
		return m_vkDescriptorSet;
	}

}// namespace LT