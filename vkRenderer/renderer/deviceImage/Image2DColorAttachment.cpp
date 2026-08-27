// 渲染对象
#include "vkRendererCommon.h"
#include "Image2DColorAttachment.h"

#include "vkContext.h"

#include "DeviceMemoryManager.h"

namespace LT {
	Image2DColorAttachment::Image2DColorAttachment(ImageID nID, vk::Format eFormat, uint32_t nWidth, uint32_t nHeight)
		:ImageViewable(nID, eFormat, nWidth, nHeight)
	{
		InitVKImage();
	}
	Image2DColorAttachment::~Image2DColorAttachment()
	{
		
	}
	void Image2DColorAttachment::InitVKImage()
	{
		vk::Device& device = vkContext::GetVkDevice();
		vk::ImageCreateInfo ici;
		ici
			.setFormat(m_eFormat)
			.setExtent(vk::Extent3D({m_nWidth, m_nHeight, 1}))
			.setArrayLayers(1)
			.setMipLevels(1)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment)
			.setTiling(vk::ImageTiling::eOptimal)
			.setImageType(vk::ImageType::e2D)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			;

		VmaAllocationCreateInfo vaci = {};
		vaci.usage = VMA_MEMORY_USAGE_AUTO;

		m_vkImage = DeviceMemoryManager::CreateImage(m_nID, ici, vaci);

		RENDERER_ASSERT(m_vkImage, "func: %s. Faild to Create Color ImageAttachment", __FUNCTION__);
	}
	void Image2DColorAttachment::AssignMemory(const void* pData, size_t nSize)
	{
		// 不赋值 仅创建ImageView

		vk::ImageSubresourceRange isr = {};
		isr
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setLayerCount(1)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			;

		vk::ImageViewCreateInfo ivci = {};

		ivci
			.setImage(m_vkImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(m_eFormat)
			.setSubresourceRange(isr)
			;

		m_vkImageView = vkContext::GetVkDevice().createImageView(ivci);
	}
}// namespace 