#include "vkRendererCommon.h"
#include "vkContext.h"
#include "Image2DDepthBuffer.h"
#include "DeviceMemoryManager.h"

namespace LT {
	Image2DDepthBuffer::Image2DDepthBuffer(ImageID id, uint32_t width, uint32_t height)
		:ImageViewable(id, vk::Format::eD32Sfloat, width, height)
	{
		InitVKImage();
		AssignMemory(nullptr, 0);
	}

	Image2DDepthBuffer::~Image2DDepthBuffer()
	{
	}

	void Image2DDepthBuffer::InitVKImage()
	{
		vk::Device& device = vkContext::GetVkDevice();

		vk::ImageCreateInfo ici;
		ici
			.setExtent(vk::Extent3D(m_nWidth, m_nHeight, 1))
			.setFormat(m_eFormat)
			.setMipLevels(1)
			.setArrayLayers(1)
			.setImageType(vk::ImageType::e2D)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			;

		m_vkImage = device.createImage(ici);

		RENDERER_ASSERT(m_vkImage, "Depth Buffer Create Failed.");
	}

	void Image2DDepthBuffer::AssignMemory(const void* pData, size_t nSize)
	{
		if (m_vkImageView)
			return;

		DeviceMemoryManager::AllocateMemory(this);

		vk::ImageSubresourceRange isr;
		isr
			.setAspectMask(vk::ImageAspectFlagBits::eDepth)
			.setLayerCount(1)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			;

		vk::ImageViewCreateInfo ivci;
		ivci
			.setImage(m_vkImage)
			.setFormat(m_eFormat)
			.setViewType(vk::ImageViewType::e2D)
			.setSubresourceRange(isr)
			;

		  m_vkImageView = vkContext::GetVkDevice().createImageView(ivci);
	}


} // namespce LT
