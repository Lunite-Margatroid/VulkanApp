#include "vkRendererCommon.h"
#include "vkContext.h"
#include "ImageManager.h"
#include "Image2DShaderRes.h"
#include "Image2DDepthBuffer.h"

namespace LT {

	ImageManager* ImageManager::s_pImageManagerInstance = nullptr;

	Image2DShaderRes* ImageManager::CreateImage2DShaderResource(vk::Format eFormat, uint32_t width, uint32_t height)
	{
		ImageManager& instance = GetInstance();
		Image2DShaderRes* pImage = new Image2DShaderRes(instance.GenImageID(), eFormat, width, height);

		instance.m_mapImage[pImage->GetImageID()] = reinterpret_cast<DeviceImage*>(pImage);

		return pImage;
	}

	Image2DDepthBuffer* ImageManager::CreateImage2DDepthBuffer(uint32_t width, uint32_t height)
	{
		ImageManager& instance = GetInstance();
		Image2DDepthBuffer* pImage = new Image2DDepthBuffer(instance.GenImageID(), width, height);

		instance.m_mapImage[pImage->GetImageID()] = reinterpret_cast<DeviceImage*>(pImage);

		return pImage;
	}

	void ImageManager::DeleteImage(DeviceImage* pImage)
	{
		ImageManager& instance = GetInstance();

		auto iter = instance.m_mapImage.find(pImage->GetImageID());
		if (iter == instance.m_mapImage.end())
		{
			LOG_WARNING("%s, the Image did not exist", __FUNCTION__);
		}
		else
		{
			delete iter->second;
			instance.m_mapImage.erase(iter);
		}
	}
	void ImageManager::Init() {
		if (!s_pImageManagerInstance)
		{
			s_pImageManagerInstance = new ImageManager();
		}
		else
		{
			LOG_WARNING("%s, Repeat Init", __FUNCTION__);
		}
	}

	void ImageManager::Release() {
		if (s_pImageManagerInstance) {
			delete s_pImageManagerInstance;
			s_pImageManagerInstance = nullptr;
		}
		else
		{
			LOG_WARNING("%s, it did not init.", __FUNCTION__);
		}
	}

	ImageManager& ImageManager::GetInstance()
	{
		if (s_pImageManagerInstance)
		{
			return *s_pImageManagerInstance;
		}
		else
		{
			LOG_WARNING("%s, ImageManager did not init. It will init implicitly now", __FUNCTION__);
			Init();
			return *s_pImageManagerInstance;
		}
	}

	ImageManager::ImageManager()
		: m_nImageIDCounter(0)
	{
		// 检查深度缓冲格式
		// 检查D32Float
		{
			vk::FormatProperties props = vkContext::GetPhysicalDevice().getFormatProperties(vk::Format::eD32Sfloat);
			RENDERER_ASSERT(
				(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment,
				"D32SFloat Does not support."
			);
		}

	}

	ImageManager::~ImageManager() {
		if (m_mapImage.size() > 0)
		{
			LOG_WARNING("%s, there are still %d images not deleted.", __FUNCTION__, static_cast<int>(m_mapImage.size()));
		}
	}

	uint64_t ImageManager::GenImageID()
	{
		return m_nImageIDCounter++;
	}

}// namespace LT
