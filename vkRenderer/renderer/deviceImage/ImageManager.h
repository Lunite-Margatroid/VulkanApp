// class ImageManager
// 位于Renderer层的图像资源管理

#pragma once

#include "DeviceImage.h"


namespace LT {
	class Image2DShaderRes;
	class Image2DDepthBuffer;
	class SwapChain;

	class ImageManager {
	private:
		union ImageHandle {
			DeviceImage* pDeviceImage;
			SwapChain* pSwapChain;

			ImageHandle() {
				pDeviceImage = nullptr;
			}

			ImageHandle(void* p) {
				pDeviceImage = reinterpret_cast<DeviceImage*>(p);
			}

			bool operator == (const ImageHandle& other) const {
				return pDeviceImage == other.pDeviceImage;
			}
		};

	private:
		int64_t m_nImageIDCounter;
		int64_t m_nSwapChainImageCounter;

		std::map<ImageID, ImageHandle> m_mapImage;

		ImageID GenImageID();

		ImageID GenSwapChainImageID();

		ImageManager();
		~ImageManager();

		DeviceImage* GetDeviceImage(ImageID id);
		SwapChain* GetSwapChain(ImageID id);

		// --------------- static ----------------
	private:
		static ImageManager* s_pImageManagerInstance;

	public:


		static Image2DShaderRes* CreateImage2DShaderResource(vk::Format eFormat, uint32_t width, uint32_t height);
		static Image2DDepthBuffer* CreateImage2DDepthBuffer(uint32_t width, uint32_t height);

		static void DeleteImage(DeviceImage* pImage);
		static void DeleteImage(ImageID nImageID);

		static void Init();
		static void Release();

		static ImageManager& GetInstance();

		static vk::Image GetNativeDeviceImage(ImageID nImageID);
		static vk::ImageView GetNativeDeviceImageView(ImageID nImageID);

		static ImageID RegisterSwapChainImage(SwapChain* pSwapChain);
		static bool UnregisterSwapChainImage(ImageID nID);
	};
}