#include "vkRendererCommon.h"
#include "vkContext.h"
#include "ImageManager.h"
#include "Image2DShaderRes.h"
#include "Image2DDepthBuffer.h"
#include "SwapChain.h"

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
			delete iter->second.pDeviceImage;
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

	DeviceImage* ImageManager::GetDeviceImage(ImageID id)
	{
#ifdef _DEBUG
		RENDERER_ASSERT(IsValidImageID(id), "Invalid Image ID.");
		RENDERER_ASSERT(m_mapImage.find(id) != m_mapImage.end(), "The Image does not exist.");
#endif // _DEBUG

		return m_mapImage[id].pDeviceImage;
	}

	SwapChain* ImageManager::GetSwapChain(ImageID id)
	{
#ifdef _DEBUG
		RENDERER_ASSERT(IsSwapChainImageID(id), "Invalid Image ID.");
		RENDERER_ASSERT(m_mapImage.find(id) != m_mapImage.end(), "The Image does not exist.");
#endif // _DEBUG

		return m_mapImage[id].pSwapChain;
	}

	ImageID ImageManager::GenImageID()
	{
		return m_nImageIDCounter++;
	}

	ImageID ImageManager::GenSwapChainImageID()
	{
		return m_nSwapChainImageCounter++;
	}

	vk::Image ImageManager::GetNativeDeviceImage(ImageID nImageID) {
		ImageManager& instance = ImageManager::GetInstance();
		auto iter = instance.m_mapImage.find(nImageID);

		if (iter != instance.m_mapImage.end())
		{
			if (IsValidImageID(nImageID))
			{
				return instance.GetDeviceImage(nImageID)->GetNativeDeviceImage();
			}

			if (IsSwapChainImageID(nImageID))
			{
				return instance.GetSwapChain(nImageID)->GetCurrentTargetImage();
			}

		}
		return VK_NULL_HANDLE;
	}

	vk::ImageView ImageManager::GetNativeDeviceImageView(ImageID nImageID) {
		ImageManager& instance = ImageManager::GetInstance();
		auto iter = instance.m_mapImage.find(nImageID);
		if (iter != instance.m_mapImage.end())
		{
			if (IsValidImageID(nImageID))
			{

				ImageViewable* pViewable = dynamic_cast<ImageViewable*>(instance.GetDeviceImage(nImageID));
				if (pViewable)
				{
					return pViewable->GetNativeImageView();
				}
			}

			if (IsSwapChainImageID(nImageID))
			{
				// Handle swapchain image
				return instance.GetSwapChain(nImageID)->GetCurrentTargetImageView();
			}

		}
		return VK_NULL_HANDLE;
	}
	ImageID ImageManager::RegisterSwapChainImage(SwapChain* pSwapChain)
	{
		ImageManager& mgr = GetInstance();
		auto & map = mgr.m_mapImage;

		ImageID id = mgr.GenSwapChainImageID();

		map[id].pSwapChain = pSwapChain;

		return id;
	}
	bool ImageManager::UnregisterSwapChainImage(ImageID nID)
	{
		bool bExist = false;
		ImageManager& mgr = GetInstance();
		auto& map = mgr.m_mapImage;


		auto iter = map.find(nID);

		if (iter != map.end())
		{
			map.erase(iter);
			bExist = true;
		} 

		return bExist;
	}
}// namespace LT
