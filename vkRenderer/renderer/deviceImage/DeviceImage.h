// Renderer的图像
// 包装vk的Image对象

#pragma once

namespace LT {


	using ImageID = uint64_t;

	class DeviceImage {

		friend class ImageManager;

	protected:
		vk::Image m_vkImage;
		ImageID m_nID;
		uint32_t m_nWidth;
		uint32_t m_nHeight;
		vk::Format m_eFormat;

		DeviceImage(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height);
		virtual ~DeviceImage();

		DeviceImage(const DeviceImage&) = delete;
		DeviceImage(DeviceImage&&) = delete;

		DeviceImage& operator = (const DeviceImage&) = delete;
		DeviceImage& operator = (DeviceImage&&) = delete;
		
		virtual void InitVKImage() = 0;

	public:

		vk::Image GetNativeDeviceImage();

		ImageID GetImageID() const;

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }
		vk::Format GetFormat() const { return m_eFormat; }

		virtual void AssignMemory(const void* pData, size_t nSize) = 0;

	};
}