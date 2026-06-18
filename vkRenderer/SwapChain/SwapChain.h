#pragma once
namespace LT {

	constexpr vk::Format SWAPCHAIN_DEFAULT_PIXEL_FORMAT = vk::Format::eR8G8B8A8Srgb;
	constexpr unsigned int SWAPCHAIN_DEFAULT_IMAGE_NUM = 2u;

	class SwapChain {
	private:
		vk::SwapchainKHR m_vkSwapChain;

		void InitSwapChain();
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


		SwapChain();
		~SwapChain();

	};
}