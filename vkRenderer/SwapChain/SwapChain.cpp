#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"

namespace LT {
	vk::SwapchainKHR& SwapChain::NativeVKSwapChain() noexcept
	{
		// TODO: 在此处插入 return 语句
		return m_vkSwapChain;
	}
	SwapChain::SwapChain()
	{
		InitSwapChain();
		CreateImageViews();
	}
	SwapChain::~SwapChain()
	{
		vkContext::GetInstance().m_vkDevice.destroySwapchainKHR(m_vkSwapChain);

		for (auto imageView : m_imageViews)
		{
			vkContext::GetInstance().m_vkDevice.destroyImageView(imageView);
		}
	}

	void SwapChain::InitSwapChain() {
		vk::SurfaceKHR surface = vkContext::GetInstance().m_vkSurface;
		assert(surface);
		// 查询所有能力的接口
		auto surfaceCapabilities = vkContext::GetInstance().m_phyDevice.getSurfaceCapabilitiesKHR(surface);
		assert(surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max());
		LOG_INFO("Width: %d  Hight: %d\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);


		// 查询支持的像素格式
		std::vector<vk::SurfaceFormatKHR> formats = vkContext::GetInstance().m_phyDevice.getSurfaceFormatsKHR(surface);
		// 默认格式
		m_sSwapChainInfo.surfaceFormat = formats[0];
		for (const auto& surfaceFormat : formats) {
			// 查询SRGB8位格式
			if (surfaceFormat.format == SWAPCHAIN_DEFAULT_PIXEL_FORMAT && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear) {
				m_sSwapChainInfo.surfaceFormat = surfaceFormat;
				break;
			}
		}

		// 查询支持的交换模式
		std::vector<vk::PresentModeKHR> presentModes = vkContext::GetInstance().m_phyDevice.getSurfacePresentModesKHR(surface);
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

		// 缓冲尺寸
		m_sSwapChainInfo.width = std::clamp<uint32_t>(
			surfaceCapabilities.currentExtent.width,
			surfaceCapabilities.minImageExtent.width,
			surfaceCapabilities.maxImageExtent.width);

		m_sSwapChainInfo.height = std::clamp<uint32_t>(
			surfaceCapabilities.currentExtent.height,
			surfaceCapabilities.minImageExtent.height,
			surfaceCapabilities.maxImageExtent.height
		);

		vk::Extent2D swapChainExtent{ static_cast<uint32_t>(m_sSwapChainInfo.width), static_cast<uint32_t>(m_sSwapChainInfo.height) };

		// 缓冲数量
		m_sSwapChainInfo.nImageCount = std::clamp<uint32_t>(SWAPCHAIN_DEFAULT_IMAGE_NUM, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

		// 创建SwapChain
		vk::SwapchainCreateInfoKHR swapCreateInfo;
		swapCreateInfo
			.setSurface(vkContext::GetInstance().m_vkSurface)				// Surface Instance
			.setMinImageCount(m_sSwapChainInfo.nImageCount)					// 缓冲数量
			.setImageExtent(swapChainExtent)								// 缓冲尺寸
			.setImageFormat(m_sSwapChainInfo.surfaceFormat.format)			// 缓冲像素格式
			.setImageColorSpace(m_sSwapChainInfo.surfaceFormat.colorSpace)	// 缓冲图像色彩空间
			.setImageArrayLayers(1)											// 每个Image包含的层数 if 不使用立体渲染 then 1
			// vr 裸眼3D之类可能需要复数个layers
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)		// eColorAttachment 直接作为渲染对象
			.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)	// 不做任何变化
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)		// 忽略Alpha直接覆盖
			.setPresentMode(m_sSwapChainInfo.presentMode)					// 交换模式
			.setClipped(true)												// if 尺寸不合 then 裁剪
			.setOldSwapchain(nullptr);										// 上一个交换链的指针 Resize后需要重新创建交换链


		// 是否允许多个Cmd Queue并行访问SwapChain
		if (vkContext::GetInstance().IsGraphicsSurfaceSameQueue())
		{
			swapCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive); // 同一时刻仅允许一个cmd Queue使用。多个Queue使用时需要显式转交所有权
			// eExclusive 性能会高一点
		}
		else
		{
			// 创建QueueForSurface成功是交换链的前置条件
			// 创建了Queue For Surface and 存在两个Queue
			swapCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent); // 同一时刻仅允许多个cmd Queue使用
		}

		m_vkSwapChain = vkContext::GetInstance().m_vkDevice.createSwapchainKHR(swapCreateInfo);
		m_sSwapChainInfo.images = vkContext::GetInstance().m_vkDevice.getSwapchainImagesKHR(m_vkSwapChain);
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
			vk::ImageView imageView =  LT::vkContext::GetInstance().m_vkDevice.createImageView(imgViewInfo);
			m_imageViews.emplace_back(std::move(imageView));
		}
	}
}