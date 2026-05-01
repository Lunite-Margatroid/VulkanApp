#pragma once
#include "vulkan\vulkan.hpp"

namespace LT {
	class vkContext {
	private:
		vk::Instance m_vkInstance;

		vkContext();
		static vkContext* s_pVkContext;
	public:
		static void Init();
		static void Release();

		~vkContext();

		static vkContext& GetInstance();
	};

}