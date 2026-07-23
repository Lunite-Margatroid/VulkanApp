#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SamplerManager.h"

namespace LT {

	SamplerManager* SamplerManager::s_pInstance = nullptr;

	SamplerManager::SamplerManager() 
		:m_nIDCounter(0)
	{
		if (vkContext::GetIsAnisotropySampleSupported())
		{
			m_fMaxAnis = vkContext::GetPhysicalDevice().getProperties().limits.maxSamplerAnisotropy;
		}
		else
		{
			m_fMaxAnis = 1.0f;
		}
	}
	SamplerManager::~SamplerManager() {
		vk::Device& device = vkContext::GetVkDevice();
		for (auto& [sci, pSampler] : m_mapImageSampler)
		{
			delete pSampler;
		}
		m_mapImageSampler.clear();
	}

	ImageSamplerID SamplerManager::GenImageSamplerID()
	{
		return m_nIDCounter ++;
	}

	void SamplerManager::Init() {
		if (s_pInstance == nullptr)
		{
			s_pInstance = new SamplerManager();
		}
		else
		{
			LOG_WARNING("%s Repeat init.", __FUNCTION__);
		}
	}

	void SamplerManager::Release() {
		if (s_pInstance)
		{
			delete s_pInstance;
		}
		else
		{
			LOG_WARNING("%s SamplerManager did not init.", __FUNCTION__);
		}
	}

	void SamplerManager::DeleteImageSampler(ImageSampler* pSampler) {
		
		RENDERER_ASSERT(s_pInstance, "SamplerManager did not init.");

		SamplerManager& samplerMgr = *s_pInstance;

		const auto& sci =  pSampler->GetSamplerCreateInfo();
		auto iter = samplerMgr.m_mapImageSampler.find(sci);
		if (iter != samplerMgr.m_mapImageSampler.end())
		{
			delete iter->second;
			samplerMgr.m_mapImageSampler.erase(iter);
		}
		else
		{
			RENDERER_ASSERT(false, "It is a wild pointer. Or A wrong creation.");
		}
	}

	ImageSampler* SamplerManager::GetDefaultImageSampler() {
		RENDERER_ASSERT(s_pInstance, "SamplerManager did not init.");

		vk::SamplerCreateInfo sci;
		sci
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setBorderColor(vk::BorderColor::eIntTransparentBlack)
			.setCompareEnable(vk::False)
			.setCompareOp(vk::CompareOp::eNever)
			.setAnisotropyEnable(vkContext::GetIsAnisotropySampleSupported() ? vk::True : vk::False)
			.setMaxAnisotropy(s_pInstance->m_fMaxAnis)
			.setUnnormalizedCoordinates(vk::False)
			.setMipLodBias(0.f)
			.setMinLod(0.f)
			.setMaxLod(0.f)
			;

		return GetImageSampler(sci);
	}

	ImageSampler* SamplerManager::GetImageSampler(const vk::SamplerCreateInfo& sci)
	{
		RENDERER_ASSERT(s_pInstance, "SamplerManager didnot init.");

		SamplerMap& samplerMap = s_pInstance->m_mapImageSampler;
		auto iter = samplerMap.find(sci);
		if (iter == samplerMap.end())
		{
			ImageSampler* pSampler = new ImageSampler(sci, s_pInstance->GenImageSamplerID());
			samplerMap[sci] = pSampler;
			return pSampler;
		}

		return iter->second;
	}

} // namespade LT