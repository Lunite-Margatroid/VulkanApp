#pragma once

namespace LT {

	constexpr unsigned int RENDERER_DEFAULT_FLIGHT_FRAME_NUM = 2u;

	class SwapChain;

	class vkContext final {

		friend class SwapChain;

	private:
		vk::Instance m_vkInstance;
	private:
		vk::PhysicalDevice m_phyDevice;
		vk::Device m_vkDevice;


		vk::SurfaceKHR m_vkSurface;
		// vk命令队列
		// 使用GetCmdQueue和GetCmdQueueForSurface获取命令队列
		vk::Queue m_vkQueue;
		vk::Queue m_vkQueueForSurface;


		// Command Pool
		vk::CommandPool m_vkCommandPool;
		std::vector<vk::CommandBuffer> m_vecCommandBuffers;


		std::unique_ptr<SwapChain> m_pSwapChain;

		// 获取的命令队列的索引 来自物理设备
		std::optional<uint32_t> m_nQueueFamilyIndex; // 支持图形的命令队列
		std::optional<uint32_t> m_nQueueIndexForSurface; // 支持sruface的命令队列

	private:
		vkContext(const std::vector<const char* >& extensions, HWND hWnd = NULL);

		void CreateVkInstance(const std::vector<const char* >& extensions);
		void PickPhyDevice();
		void CreateVkDevice();
		void CreateSurface(HWND hWnd = NULL);

		void CreateCommandPool();
		void CreateCommandBuffer();



	public:
		// 找到了一个同时支持Surface和Graphics的Queue且只创建了一个Queue
		inline bool IsGraphicsSurfaceSameQueue() const noexcept {
			return m_nQueueFamilyIndex.has_value() && \
				m_nQueueIndexForSurface.has_value() && \
				m_nQueueFamilyIndex.value() == m_nQueueIndexForSurface.value();
		}
		~vkContext();

		// ------------ 静态 ----------------------
	private:
		static vkContext* s_pVkContext;
	public:
		static void Init(const std::vector<const char* >& extensions, HWND hWnd = NULL);
		static void Release();
		static vkContext& GetInstance();
		static vk::Device& GetVkDevice();
		static vk::PhysicalDevice& GetPhysicalDevice();
		static vk::SwapchainKHR& GetNativeSwapChain();
		static SwapChain& GetSwapChain();


		static void InitSwapChain();
		static void ReleaseSwapChain();

		static void WaitIdel();

		static void ResizeSwapChain(unsigned int width, unsigned int height);

		static vk::Queue& GetCmdQueue();
		static vk::Queue& GetCmdQueueForSurface();
		static vk::CommandPool& GetCmdPool();

		static vk::CommandBuffer& GetCmdBuffer(unsigned int nIndex);
	};

}