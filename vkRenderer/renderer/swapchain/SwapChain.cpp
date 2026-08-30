#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"

namespace LT {
	vk::SwapchainKHR& SwapChain::NativeVKSwapChain() noexcept
	{
		// TODO: 在此处插入 return 语句
		return m_vkSwapChain;
	}
	SwapChain::SwapChain(uint32_t nWidth, uint32_t nHeight, vk::Device device, vk::PhysicalDevice vkPhyDeivce, vk::SurfaceKHR vkSurface, vk::SharingMode eImageShaderingMode)
		:m_vkDevice(device)
		,m_vkPhyDevice(vkPhyDeivce)
		,m_vkSurface(vkSurface)
	{
		m_vkSwapCreateInfo.setImageSharingMode(eImageShaderingMode);
		InitSwapChain(nWidth, nHeight);
		CreateImageViews();
	}
	SwapChain::~SwapChain()
	{
		m_vkDevice.destroySwapchainKHR(m_vkSwapChain);

		for (auto imageView : m_imageViews)
		{
			m_vkDevice.destroyImageView(imageView);
		}
	}

	void SwapChain::InitSwapChain(uint32_t nWidth, uint32_t nHeight) {

		RENDERER_ASSERT(m_vkSurface, "%s", __FUNCTION__);
		// 查询所有能力的接口
		auto surfaceCapabilities = m_vkPhyDevice.getSurfaceCapabilitiesKHR(m_vkSurface);
		RENDERER_ASSERT(surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max(), "%s", __FUNCTION__);
		LOG_INFO("Width: %d  Hight: %d\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);


		RENDERER_ASSERT(surfaceCapabilities.minImageExtent.width <= surfaceCapabilities.maxImageExtent.width, "Func: %s. clampFailed. clamp(%u, %d)", 
			__FUNCTION__, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);

		RENDERER_ASSERT(surfaceCapabilities.minImageExtent.height <= surfaceCapabilities.maxImageExtent.height, "Func: %s. clampFailed. clamp(%u, %d)", 
			__FUNCTION__, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);


		// 缓冲尺寸
		m_sSwapChainInfo.width = std::clamp<uint32_t>(
			nWidth,
			surfaceCapabilities.minImageExtent.width,
			surfaceCapabilities.maxImageExtent.width
		);

		m_sSwapChainInfo.height = std::clamp<uint32_t>(
			nHeight,
			surfaceCapabilities.minImageExtent.height,
			surfaceCapabilities.maxImageExtent.height
		);

		

		if (surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0)
			return;

		// 查询支持的像素格式
		std::vector<vk::SurfaceFormatKHR> formats = m_vkPhyDevice.getSurfaceFormatsKHR(m_vkSurface);
		// 默认格式
		m_sSwapChainInfo.surfaceFormat = formats[0];
		for (const auto& surfaceFormat : formats) {
			// 查询SRGB8位格式
			if (surfaceFormat.format == SWAPCHAIN_DEFAULT_PIXEL_FORMAT && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				m_sSwapChainInfo.surfaceFormat = surfaceFormat;
				break;
			}
		}

		// 查询支持的交换模式
		std::vector<vk::PresentModeKHR> presentModes = m_vkPhyDevice.getSurfacePresentModesKHR(m_vkSurface);
		// vk::PresentModeKHR::eFifo为一定支持的格式
		m_sSwapChainInfo.presentMode = vk::PresentModeKHR::eFifo;
		for (const auto& presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				m_sSwapChainInfo.presentMode = presentMode;
				break;
			}
		}

		vk::Extent2D swapChainExtent{ static_cast<uint32_t>(m_sSwapChainInfo.width), static_cast<uint32_t>(m_sSwapChainInfo.height) };

		// 缓冲数量		
		if(surfaceCapabilities.minImageCount <= surfaceCapabilities.maxImageCount)
		{
			m_sSwapChainInfo.nImageCount = std::clamp<uint32_t>(SWAPCHAIN_DEFAULT_IMAGE_NUM, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
		}
		else
		{
			m_sSwapChainInfo.nImageCount = SWAPCHAIN_DEFAULT_IMAGE_NUM;
		}
		

		// 创建SwapChain
		m_vkSwapCreateInfo
			.setSurface(m_vkSurface)				// Surface Instance
			.setMinImageCount(m_sSwapChainInfo.nImageCount)					// 缓冲数量
			.setImageExtent(swapChainExtent)								// 缓冲尺寸
			.setImageFormat(m_sSwapChainInfo.surfaceFormat.format)			// 缓冲像素格式
			.setImageColorSpace(m_sSwapChainInfo.surfaceFormat.colorSpace)	// 缓冲图像色彩空间
			.setImageArrayLayers(1)											// 每个Image包含的层数 if 不使用立体渲染 then 1
			// vr 裸眼3D之类可能需要复数个layers
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)		// eColorAttachment 直接作为渲染对象
			.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)	// 不变
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)		// 忽略Alpha直接覆盖
			.setPresentMode(m_sSwapChainInfo.presentMode)					// 交换模式
			.setClipped(true)												// if 尺寸不合 then 裁剪
			.setOldSwapchain(nullptr);										// 上一个交换链的指针 Resize后需要重新创建交换链

		m_vkSwapChain = m_vkDevice.createSwapchainKHR(m_vkSwapCreateInfo);
		m_sSwapChainInfo.images = m_vkDevice.getSwapchainImagesKHR(m_vkSwapChain);
	}

	void SwapChain::CreateImageViews() {
		LOG_TRACING("Create Image Views");
		for (auto image : m_sSwapChainInfo.images)
		{
			vk::ImageViewCreateInfo imgViewInfo;
			vk::ImageSubresourceRange subRes;
			vk::ComponentMapping componentMapping;

			subRes
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor);
			imgViewInfo
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(m_sSwapChainInfo.surfaceFormat.format)
				.setSubresourceRange(subRes)
				.setComponents(componentMapping)
				.setImage(image);
			vk::ImageView imageView = m_vkDevice.createImageView(imgViewInfo);
			m_imageViews.emplace_back(std::move(imageView));
		}
	}

	void SwapChain::Resize(uint32_t nWidth, uint32_t nHeight) {
		// 删除ImageView
		for (auto& imageView : m_imageViews)
		{
			m_vkDevice.destroyImageView(imageView);
		}
		m_imageViews.clear();

		// 删除vkSwapChain
		m_vkDevice.destroySwapchainKHR(m_vkSwapChain);


		// 缓冲尺寸
		m_sSwapChainInfo.width = nWidth;
		m_sSwapChainInfo.height = nHeight;

		auto surfaceCapabilities = m_vkPhyDevice.getSurfaceCapabilitiesKHR(m_vkSurface);
		RENDERER_ASSERT(surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max(), "%s", __FUNCTION__);

		RENDERER_ASSERT(surfaceCapabilities.minImageExtent.width <= surfaceCapabilities.maxImageExtent.width, "%s", __FUNCTION__);

		// 缓冲尺寸
		m_sSwapChainInfo.width = std::clamp<uint32_t>(
			nWidth,
			surfaceCapabilities.minImageExtent.width,
			surfaceCapabilities.maxImageExtent.width
		);

		m_sSwapChainInfo.height = std::clamp<uint32_t>(
			nHeight,
			surfaceCapabilities.minImageExtent.height,
			surfaceCapabilities.maxImageExtent.height
		);



		vk::Extent2D swapChainExtent{ static_cast<uint32_t>(m_sSwapChainInfo.width), static_cast<uint32_t>(m_sSwapChainInfo.height) };

		// 创建SwapChain
		m_vkSwapCreateInfo
			.setImageExtent(swapChainExtent)// 缓冲尺寸
			;

		m_vkSwapChain = m_vkDevice.createSwapchainKHR(m_vkSwapCreateInfo);
		m_sSwapChainInfo.images = m_vkDevice.getSwapchainImagesKHR(m_vkSwapChain);

		CreateImageViews();
	}

	vk::Image SwapChain::GetCurrentTargetImage() {
		return m_sSwapChainInfo.images[m_nCurrentImageIndex % m_sSwapChainInfo.nImageCount];
	}

	vk::ImageView SwapChain::GetCurrentTargetImageView() {
		return m_imageViews[m_nCurrentImageIndex % m_sSwapChainInfo.nImageCount];
	}
}