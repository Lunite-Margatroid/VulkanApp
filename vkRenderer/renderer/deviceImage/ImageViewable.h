#pragma once
#include "DeviceImage.h"

namespace LT {
	class ImageViewable : public DeviceImage {
	protected:
		vk::ImageView m_vkImageView;

		ImageViewable(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height);
		~ImageViewable();
	public:
		vk::ImageView GetNativeImageView();

	};
	

} // namespace LT