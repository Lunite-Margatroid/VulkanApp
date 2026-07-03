#include "vkRendererCommon.h"
#include "vkContext.h"
#include "Image2DShaderRes.h"
#include "DeviceMemoryManager.h"

namespace LT {
	Image2DShaderRes::Image2DShaderRes(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height)
		:DeviceImage(id, eFormat, width, height)
	{
		InitVKImage();
	}
	Image2DShaderRes::~Image2DShaderRes()
	{
		DeviceMemoryManager::FreeImageMemory(*this);
	}
	void Image2DShaderRes::InitVKImage()
	{
		vk::Device& device = vkContext::GetVkDevice();
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

		m_vkImage = device.createImage(ici);

		RENDERER_ASSERT(m_vkImage, "Failed to create vk::Image for Image2DShaderRes");
	}
	void Image2DShaderRes::AssignMemory(void* pData, size_t nSize)
	{
		RENDERER_ASSERT(pData, "pData is nullptr in AssignMemory for Image2DShaderRes");

		DeviceMemoryManager::AllocateMemory(this);

		DeviceMemoryManager::AsignMemory(this, nSize, pData);
	}
} // namespace LT