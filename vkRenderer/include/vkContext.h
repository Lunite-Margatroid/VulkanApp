#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.hpp"

namespace LT {

	class SwapChain;

	class vkContext final{
	private:
		vk::Instance m_vkInstance;
	public:
		vk::PhysicalDevice m_phyDevice;
		vk::Device m_vkDevice;

		vk::SurfaceKHR m_vkSurface;
		// vk命令队列
		// 使用GetCmdQueue和GetCmdQueueForSurface获取命令队列
		vk::Queue m_vkQueue;
		vk::Queue m_vkQueueForSurface;

		std::unique_ptr<SwapChain> m_pSwapChain;

		// 获取的命令队列的索引 来自物理设备
		std::optional<uint32_t> m_nQueueFamilyIndex;
		std::optional<uint32_t> m_nQueueIndexForSurface;
	private:
		vkContext(const std::vector<const char* >& extensions, HWND hWnd = NULL);
		
		void CreateVkInstance(const std::vector<const char* >& extensions);
		void PickPhyDevice();
		void CreateVkDevice();
		void CreateSurface(HWND hWnd = NULL);

	public:
		// 找到了一个同时支持Surface和Graphics的Queue且只创建了一个Queue
		inline bool IsGraphicsSurfaceSameQueue() const noexcept;

		inline vk::Queue GetCmdQueue() noexcept;
		inline vk::Queue GetCmdQueueForSurface()noexcept;



		~vkContext();

		// ------------ 静态 ----------------------
	private:
		static vkContext* s_pVkContext;
	public:
		static void Init(const std::vector<const char* >& extensions, HWND hWnd = NULL);
		static void Release();
		inline static vkContext& GetInstance() noexcept;
		inline static vk::Device& GetVkDevice() noexcept;

		static void InitSwapChain();
		static void ReleaseSwapChain();
	};

}