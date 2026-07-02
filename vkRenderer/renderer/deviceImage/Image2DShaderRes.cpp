#include "vkRendererCommon.h"
#include "vkContext.h"
#include "Image2DShaderRes.h"

namespace LT {
	Image2DShaderRes::Image2DShaderRes(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height)
		:DeviceImage(id, eFormat, width, height)
	{
	}
	Image2DShaderRes::~Image2DShaderRes()
	{
	}
	void Image2DShaderRes::InitVKImage()
	{
		vk::ImageCreateInfo ici;
		ici
			.setFormat(m_eFormat)
			.setExtent(vk::Extent3D{m_nWidth, m_nHeight, 1})
			.setArrayLayers(1)
			.setMipLevels(1)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
			.setTiling(vk::ImageTiling::eOptimal)
			.setImageType(vk::ImageType::e2D)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			;
	}
} // namespace LT