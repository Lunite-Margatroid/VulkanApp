#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"

namespace LT {
	SwapChain::SwapChain()
	{
		vk::SurfaceKHR surface = vkContext::GetInstance().m_vkSurface;
		assert(surface);
		// 查询所有属性的接口
		// auto surfaceCapabilities = vkContext::GetInstance().m_phyDevice.getSurfaceCapabilitiesKHR(surface);
		
		// 查询支持的像素格式
		std::vector<vk::SurfaceFormatKHR> formats = vkContext::GetInstance().m_phyDevice.getSurfaceFormatsKHR(surface);
		m_sSwapChainInfo.surfaceFormat = formats[0];
		for (const auto& surfaceFormat : formats) {
			if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear) {
				m_sSwapChainInfo.surfaceFormat = surfaceFormat;
				break;
			}
		}

		// 查询支持的交换模式
		std::vector<vk::PresentModeKHR> presentModes = vkContext::GetInstance().m_phyDevice.getSurfacePresentModesKHR(surface);
		m_sSwapChainInfo.presentMode = vk::PresentModeKHR::eFifo;
		for (const auto& presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				m_sSwapChainInfo.presentMode = presentMode;
				break;
			}
		}

	}
}