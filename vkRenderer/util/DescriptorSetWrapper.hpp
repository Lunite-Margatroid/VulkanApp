#pragma once
#include "FenceManager.hpp"

namespace LT {
//┌───────────────────────┐
//│ wait for last submit  │
//└──────────┬────────────┘
//           │             
//┌──────────▼────────────┐
//│ UpdateDescriptorSet   │
//└─────────┬─────────────┘
//          │              
//          ▼              
// ┌────────────────────┐  
// │ bind to pipeline   │  
// └────────┬───────────┘  
//          │              
// ┌────────▼───────────┐  
// │   front->SetFence  │  
// └─────────┬──────────┘  
//           │             
//┌──────────▼───────────┐ 
//│   push front to back │ 
//└──────────┬───────────┘ 
//           │             
//  ┌────────▼────────────┐
//  │      pop front      │
//  └─────────────────────┘

	class DescriptorSetWrapper {
	private:
		struct DescriptorSetWithFence {
			vk::DescriptorSet vkDescriptorSet;
			FenceRef refFenceDrawing;
			DescriptorSetWithFence()
				: vkDescriptorSet(VK_NULL_HANDLE)
				, refFenceDrawing(INVALID_FENCE_HANDLE)
			{}
		};

		std::array<DescriptorSetWithFence, RENDERER_DESCRIPTOR_SET_COUNT> m_arrDescriptorSets;

		std::queue<size_t> m_qDescriptorSets;

	private:
		DescriptorSetWithFence& Front() { return m_arrDescriptorSets[m_qDescriptorSets.front()]; }

	public:
		DescriptorSetWrapper();
		DescriptorSetWrapper(const DescriptorSetWrapper&) = delete;
		DescriptorSetWrapper(DescriptorSetWrapper&&) = delete;
		DescriptorSetWrapper& operator = (const DescriptorSetWrapper&) = delete;
		DescriptorSetWrapper& operator = (DescriptorSetWrapper&&) = delete;

		// 这里控制权交给DescriptorSetWrapper
		// 调用Destroy释放
		void Init(vk::DescriptorPool vkDescriptorPool, const vk::DescriptorSetLayout& sDescriptorSet);
		void Destroy(vk::DescriptorPool vkDescriptorPool);
		// 该函数会等待Fence
		vk::DescriptorSet GetNativeDescriptorSet();
		vk::DescriptorSet GetNativeDescriptorSetWithoutWaiting();
		void WaitForFenceDrawing();
		FenceRef GetFenceDrawing();
		void SetFenceDrawing(const FenceRef& refFence);

		void UpdateDescriptorSets(std::vector<vk::WriteDescriptorSet>& descriptorWrites);

		void PopFrontAndPushBack();

		operator bool() const;
	};
}// namespace LT