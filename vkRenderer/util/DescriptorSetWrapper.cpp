#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DescriptorSetWrapper.hpp"
namespace LT {
	DescriptorSetWrapper::DescriptorSetWrapper() 
		: m_vkDescriptorSet(VK_NULL_HANDLE)
		, m_refFenceDrawing(INVALID_FENCE_HANDLE)
	{}

	DescriptorSetWrapper::DescriptorSetWrapper(vk::DescriptorSet vkDescriptorSet)
	{
		Init(vkDescriptorSet);
	}
	void DescriptorSetWrapper::Init(vk::DescriptorSet vkDescriptorSet) {
		m_vkDescriptorSet = vkDescriptorSet;
		m_refFenceDrawing = FenceRef(INVALID_FENCE_HANDLE);
	}

	void DescriptorSetWrapper::Destroy(vk::DescriptorPool vkDescriptorPool)
	{
		vk::Device &device = vkContext::GetVkDevice();
		std::array<vk::DescriptorSet, 1> arrDescSet{m_vkDescriptorSet};
		device.freeDescriptorSets(vkDescriptorPool, arrDescSet);

		Init(VK_NULL_HANDLE);
	}

	vk::DescriptorSet DescriptorSetWrapper::GetNativeDescriptorSet()
	{
		vk::Device& device = vkContext::GetVkDevice();
		if (m_refFenceDrawing)
		{
			vk::Result result = device.waitForFences(m_refFenceDrawing.GetNativeFence(), vk::True, std::_Max_limit<uint64_t>());

			RENDERER_ASSERT(result == vk::Result::eSuccess, "FenceDrawing of Descriptor: Wait failed.");
			device.resetFences(m_refFenceDrawing.GetNativeFence());
		}

		return m_vkDescriptorSet;
	}
	vk::DescriptorSet DescriptorSetWrapper::GetNativeDescriptorSetWithoutWaiting()
	{
		return m_vkDescriptorSet;
	}
	FenceRef DescriptorSetWrapper::GetFenceDrawing()
	{
		return m_refFenceDrawing;
	}

	void DescriptorSetWrapper::SetFenceDrawing(const FenceRef& refFence)
	{
		m_refFenceDrawing = refFence;
	}

	DescriptorSetWrapper::operator bool() const{
		return m_vkDescriptorSet;
	}

}// namespace LT