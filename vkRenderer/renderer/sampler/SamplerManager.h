#pragma once

namespace LT {

	class ImageSampler;

	class SamplerManager {
	
	private:
		static SamplerManager* m_pInstance;

	public:
		static void Init();
		static void Release();

		static void CreateImageSampler();
		static void ReleaseImageSampler();
	};


} // namespace LT