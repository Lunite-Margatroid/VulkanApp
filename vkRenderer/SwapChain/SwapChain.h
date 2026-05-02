#pragma once
namespace LT {
	class SwapChain {

	private:

	public:
		struct SwapChainInfo {
			vk::SurfaceFormatKHR surfaceFormat;
			vk::PresentModeKHR presentMode;
		}
		m_sSwapChainInfo;


		SwapChain();
	};
}