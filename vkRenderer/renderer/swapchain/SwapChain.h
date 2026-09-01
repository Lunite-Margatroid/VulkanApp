#pragma once
namespace LT {

	constexpr vk::Format SWAPCHAIN_DEFAULT_PIXEL_FORMAT = vk::Format::eR8G8B8A8Srgb;
	constexpr unsigned int SWAPCHAIN_DEFAULT_IMAGE_NUM = 2u;

	class SwapChain {
	private:
		vk::SwapchainKHR m_vkSwapChain;

		// 下面三个对象的生命周期不由SwapChain管理
		vk::PhysicalDevice m_vkPhyDevice;
		vk::Device m_vkDevice;
		vk::SurfaceKHR m_vkSurface;

		vk::SwapchainCreateInfoKHR m_vkSwapCreateInfo = {};

		uint32_t m_nCurrentImageIndex = 0;

		void InitSwapChain(uint32_t nWidth, uint32_t nHeight);
		void CreateImageViews();

	public:
		struct SwapChainInfo {
			vk::SurfaceFormatKHR surfaceFormat;
			vk::PresentModeKHR presentMode;
			int width, height;
			int nImageCount;
			std::vector<vk::Image> images;
			SwapChainInfo()
				:presentMode(vk::PresentModeKHR::eFifo), width(0),height(0),nImageCount(0)
			{}
		}
		m_sSwapChainInfo;

		std::vector<vk::ImageView> m_imageViews;

		vk::SwapchainKHR& NativeVKSwapChain() noexcept;

		void Resize(uint32_t nWidth, uint32_t nHeight);

		vk::Image GetCurrentTargetImage();
		vk::ImageView GetCurrentTargetImageView();

		int32_t AcquireNextImage(uint64_t nTimeOut, vk::Semaphore sem, vk::Fence fence);

		SwapChain(uint32_t nWidth, uint32_t nHeight, vk::Device device, vk::PhysicalDevice vkPhyDeivce, vk::SurfaceKHR Surface, vk::SharingMode eImageShaderingMode = vk::SharingMode::eConcurrent);
		~SwapChain();

	};
}