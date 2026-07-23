#pragma once
#include "IBindable.h"


namespace LT {
	using ImageSamplerID = int64_t;

	class ImageSampler : public IBindable{
		friend class SamplerManager;
	protected:
		vk::SamplerCreateInfo m_vkSamplerCreateInfo;
		vk::Sampler m_vkSampler;
		ImageSamplerID m_id;


	protected:
		ImageSampler(const vk::SamplerCreateInfo& sci, ImageSamplerID id);
		~ImageSampler();

		ImageSampler(const ImageSampler&) = delete;
		ImageSampler(ImageSampler&&) = delete;

		ImageSampler& operator = (const ImageSampler&) = delete;
		ImageSampler& operator = (ImageSampler&&) = delete;

	public:
		vk::Sampler GetNativeSampler();
		ImageSamplerID GetID() const;

		const vk::SamplerCreateInfo& GetSamplerCreateInfo() const;

		void Bind(BindTarget nTarget) override;
		void Unbind() override;
	};

} // namespace LT