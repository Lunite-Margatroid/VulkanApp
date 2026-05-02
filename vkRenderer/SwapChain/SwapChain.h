#pragma once
namespace LT {
	class SwapChain {
	private:
		vk::SwapchainKHR m_vkSwapChain;
	public:
		struct SwapChainInfo {
			vk::SurfaceFormatKHR surfaceFormat;
			vk::PresentModeKHR presentMode;
			int width, height;
			int nImageCount;
			std::vector<vk::Image> images;
		}
		m_sSwapChainInfo;


		SwapChain();
		~SwapChain();
	};
}