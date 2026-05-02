#pragma once
#include "vulkan\vulkan.hpp"

namespace LT {
	class vkContext {
	private:
		vk::Instance m_vkInstance;
		vk::PhysicalDevice m_phyDevice;

		vkContext();
		static vkContext* s_pVkContext;


		void PickPhyDevice();
	public:
		static void Init();
		static void Release();

		~vkContext();

		static vkContext& GetInstance();
	};

}