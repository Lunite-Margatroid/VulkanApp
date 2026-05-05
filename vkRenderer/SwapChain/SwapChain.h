#pragma once
namespace LT {
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
		}
		m_sSwapChainInfo;

		std::vector<vk::ImageView> m_imageViews;


		SwapChain();
		~SwapChain();
	};
}