#include "vkRendererCommon.h"
#include "vkContext.h"
#include "ImageSampler.h"


namespace LT {
	ImageSampler::ImageSampler(const vk::SamplerCreateInfo& sci, ImageSamplerID id)
		: m_vkSamplerCreateInfo(sci)
		, m_id(id)
	{
		m_vkSampler = vkContext::GetVkDevice().createSampler(sci);
	}

	ImageSampler::~ImageSampler() {
		vkContext::GetVkDevice().destroySampler(m_vkSampler);
	}


	vk::Sampler ImageSampler::GetNativeSampler() {
		return m_vkSampler;
	}

	ImageSamplerID ImageSampler::GetID() const {
		return m_id;
	}

	const vk::SamplerCreateInfo& ImageSampler::GetSamplerCreateInfo() const {
		return m_vkSamplerCreateInfo;
	}

	void ImageSampler::Bind(BindTarget nTarget) {
		// TODO:
	}

	void ImageSampler::Unbind() {
		// TODO:
	}

} // namespace LT