#include "vkRendererCommon.h"
#include "vkContext.h"
#include "ImageViewable.h"
#include "DeviceMemoryManager.h"

namespace LT {
	ImageViewable::ImageViewable(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height)
		:DeviceImage(id, eFormat, width, height)
	{
	}
	ImageViewable::~ImageViewable()
	{
		vkContext::GetVkDevice().destroyImageView(m_vkImageView);
		DeviceMemoryManager::FreeImageMemory(*this);
	}
	vk::ImageView ImageViewable::GetNativeImageView()
	{
		return m_vkImageView;
	}
} // namespace LT
