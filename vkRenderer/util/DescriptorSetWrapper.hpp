#pragma once
#include "FenceManager.hpp"

namespace LT {
	class DescriptorSetWrapper {
	private:
		vk::DescriptorSet m_vkDescriptorSet;
		FenceRef m_refFenceDrawing;
	public:
		DescriptorSetWrapper();
		explicit DescriptorSetWrapper(vk::DescriptorSet vkDescriptorSet);
		void Init(vk::DescriptorSet vkDescriptorSet);
		void Destroy(vk::DescriptorPool vkDescriptorPool);
		// 该函数会等待Fence
		vk::DescriptorSet GetNativeDescriptorSet();
		vk::DescriptorSet GetNativeDescriptorSetWithoutWaiting();
		void WaitForFenceDrawing();
		FenceRef GetFenceDrawing();
		void SetFenceDrawing(const FenceRef& refFence);

		operator bool() const;
	};
}// namespace LT