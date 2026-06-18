#pragma once
#define VK_USE_PLATFORM_WIN32_KHR

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#include "vulkan\vulkan.hpp"
#include "vulkan\vulkan_raii.hpp"

namespace LT {

	constexpr unsigned int RENDERER_DEFAULT_FLIGHT_FRAME_NUM = 2u;

	class SwapChain;

	class vkContext final{

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


		// debug shader
		vk::ShaderModule m_vkDebugShaderMod;

		vk::PipelineLayout m_vkDebugPipelineLayout;
		vk::Pipeline m_vkDebugPipeline;

		std::vector<vk::Semaphore> m_vkSemRenderFinish;	// 数量与swapchain的image一致
		std::vector<vk::Semaphore> m_vkSemPresentComplete;	// 数量与flight frame一致

		std::vector<vk::Fence> m_vkFenceDraw;	// 数量与flight frame一致

		uint64_t m_nFrameCount;

	private:
		vkContext(const std::vector<const char* >& extensions, HWND hWnd = NULL);
		
		void CreateVkInstance(const std::vector<const char* >& extensions);
		void PickPhyDevice();
		void CreateVkDevice();
		void CreateSurface(HWND hWnd = NULL);

		void CreateDebugShaderModule();

		void CreateDebugPipeline();


		void CreateCommandPool();
		void CreateCommandBuffer();

		void CreateDebugSyncObjects();



		void RecordCommandBufferDebug(unsigned int imageIndex, unsigned int nFrameIndex);

		void TransitionImageLayout(
			uint32_t nImageIndex,
			uint32_t nFrameIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessFlag,
			vk::AccessFlags2 dstAccessFlag,
			vk::PipelineStageFlags2 srcStageFlag,
			vk::PipelineStageFlags2 dstStageFlag
		);


	public:
		// 找到了一个同时支持Surface和Graphics的Queue且只创建了一个Queue
		inline bool IsGraphicsSurfaceSameQueue() const noexcept;

		inline vk::Queue& GetCmdQueue() noexcept;
		inline vk::Queue& GetCmdQueueForSurface()noexcept;

		void DrawFrameDebug();

		~vkContext();

		// ------------ 静态 ----------------------
	private:
		static vkContext* s_pVkContext;
	public:
		static void Init(const std::vector<const char* >& extensions, HWND hWnd = NULL);
		static void Release();
		inline static vkContext& GetInstance() noexcept;
		inline static vk::Device& GetVkDevice() noexcept;
		static vk::SwapchainKHR& GetSwapChain() noexcept;


		static void InitSwapChain();
		static void ReleaseSwapChain();

		static void DebugFrame();

		static void WaitIdel();

		static void ResizeSwapChain(unsigned int width, unsigned int height);
	};

}