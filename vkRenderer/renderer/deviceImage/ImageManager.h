// class ImageManager
// 位于Renderer层的图像资源管理

#pragma once

#include "DeviceImage.h"


namespace LT {
	class Image2DShaderRes;

	class ImageManager {
	private:
		uint64_t m_nImageIDCounter;

		std::map<ImageID, DeviceImage*> m_mapImage;

		uint64_t GenImageID();

		ImageManager();
		~ImageManager();

	private:
		static ImageManager* s_pImageManagerInstance;

	public:


		static Image2DShaderRes* CreateImage2DShaderResource(vk::Format eFormat, uint32_t width, uint32_t height);
		static void DeleteImage(DeviceImage* pImage);

		static void Init();
		static void Release();

		static ImageManager& GetInstance();
	};
}