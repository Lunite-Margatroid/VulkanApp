#pragma once

namespace LT {
	class DescriptorSetWrapper {
	private:
		vk::DescriptorSet m_vkDescriptorSet;
		vk::Fence m_fenceDrawing;
	public:
		DescriptorSetWrapper();
		DescriptorSetWrapper(vk::DescriptorSet vkDescriptorSet);
		void Init(vk::DescriptorSet vkDescriptorSet);
		void Destroy(vk::DescriptorPool vkDescriptorPool);
		// 该函数会等待Fence
		vk::DescriptorSet GetNativeDescriptorSet();
		vk::DescriptorSet GetNativeDescriptorSetWithoutWaiting();
		vk::Fence GetFenceDrawing();

		operator bool() const;
	};
}// namespace LT