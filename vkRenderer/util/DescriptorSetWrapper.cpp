#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DescriptorSetWrapper.hpp"
namespace LT {
	DescriptorSetWrapper::DescriptorSetWrapper() 
	{}

	void DescriptorSetWrapper::Init(vk::DescriptorPool vkDescriptorPool, const vk::DescriptorSetLayout& sDescriptorSet) {
		vk::Device& device = vkContext::GetVkDevice();

		std::array<vk::DescriptorSetLayout, RENDERER_DESCRIPTOR_SET_COUNT> layouts;
		layouts.fill(sDescriptorSet);

		vk::DescriptorSetAllocateInfo dsai = {};
		dsai
			.setDescriptorPool(vkDescriptorPool)
			.setSetLayouts(layouts)
			;
		std::vector<vk::DescriptorSet> vecDescriptorSets = device.allocateDescriptorSets(dsai);
		RENDERER_ASSERT(m_arrDescriptorSets.size() == vecDescriptorSets.size(), "out of bound");
		for (int i = 0; i < m_arrDescriptorSets.size(); ++i) {
			m_arrDescriptorSets[i].vkDescriptorSet = vecDescriptorSets[i];
			m_arrDescriptorSets[i].refFenceDrawing = FenceRef(INVALID_FENCE_HANDLE);
			m_qDescriptorSets.push(i);
		}

	}

	void DescriptorSetWrapper::Destroy(vk::DescriptorPool vkDescriptorPool)
	{
		vk::Device &device = vkContext::GetVkDevice();
		std::array<vk::DescriptorSet, RENDERER_DESCRIPTOR_SET_COUNT> arrDescSet;
		for (int i = 0; i < m_arrDescriptorSets.size(); i++)
		{
			arrDescSet[i] = m_arrDescriptorSets[i].vkDescriptorSet;
			m_arrDescriptorSets[i].refFenceDrawing = FenceRef(INVALID_FENCE_HANDLE);
		}

		device.freeDescriptorSets(vkDescriptorPool, arrDescSet);
	}

	vk::DescriptorSet DescriptorSetWrapper::GetNativeDescriptorSet()
	{
		WaitForFenceDrawing();
		return GetNativeDescriptorSetWithoutWaiting();
	}
	vk::DescriptorSet DescriptorSetWrapper::GetNativeDescriptorSetWithoutWaiting()
	{
		if (m_qDescriptorSets.empty())
		{
			return VK_NULL_HANDLE;
		}
		else
		{
			return Front().vkDescriptorSet;
		}
	}
	void DescriptorSetWrapper::WaitForFenceDrawing()
	{
		if (m_qDescriptorSets.empty())
		{
			// pass
		}
		else
		{
			Front().refFenceDrawing.WaitFence();
		}
	}
	FenceRef DescriptorSetWrapper::GetFenceDrawing()
	{
		if (m_qDescriptorSets.empty())
		{
			return FenceRef(INVALID_FENCE_HANDLE);
		}
		else
		{
			return Front().refFenceDrawing;
		}
	}

	void DescriptorSetWrapper::SetFenceDrawing(const FenceRef& refFence)
	{
		if(!m_qDescriptorSets.empty())
		{
			Front().refFenceDrawing = refFence;
		}
	}

	DescriptorSetWrapper::operator bool() const{
		return !m_qDescriptorSets.empty();
	}

	void DescriptorSetWrapper::UpdateDescriptorSets(std::vector<vk::WriteDescriptorSet>& descriptorWrites)
	{
		for (vk::WriteDescriptorSet& writeInfo : descriptorWrites)
		{
			writeInfo.setDstSet(Front().vkDescriptorSet);
		}
		vkContext::GetVkDevice().updateDescriptorSets(
			descriptorWrites.size(), 
			descriptorWrites.data(),
			0,
			nullptr);
	}

	void DescriptorSetWrapper::PopFrontAndPushBack()
	{
		if (!m_qDescriptorSets.empty())
		{
			size_t index = m_qDescriptorSets.front();
			m_qDescriptorSets.pop();
			m_qDescriptorSets.push(index);
		}
	}

}// namespace LT