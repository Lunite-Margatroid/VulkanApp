#pragma once

#include "ImageSampler.h"

namespace LT {
	// 定义Hash SamplerCreateInfo的仿函数
	struct SamplerCreateInfoHash {
		size_t operator()(const vk::SamplerCreateInfo& info) const noexcept {
			uint64_t seed = 0x9e3779b97f4a7c15ULL; // 黄金比例常数

			// 组合所有影响采样器创建的字段
			auto hash_combine = [&seed](auto value) {
				seed ^= static_cast<uint64_t>(value) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
				};

			// 基础过滤参数
			hash_combine(info.magFilter);
			hash_combine(info.minFilter);
			hash_combine(info.mipmapMode);

			// 寻址模式
			hash_combine(info.addressModeU);
			hash_combine(info.addressModeV);
			hash_combine(info.addressModeW);

			// LOD 相关
			hash_combine(std::bit_cast<uint32_t>(info.mipLodBias)); // float转换位模式
			hash_combine(std::bit_cast<uint32_t>(info.minLod));
			hash_combine(std::bit_cast<uint32_t>(info.maxLod));

			// 各向异性
			hash_combine(info.anisotropyEnable ? vk::True : vk::False);            // 格式化Bool32
			hash_combine(std::bit_cast<uint32_t>(info.maxAnisotropy));

			// 比较功能
			hash_combine(info.compareEnable ? vk::True : vk::False); // 格式化Bool32
			hash_combine(info.compareOp);

			// 边界颜色和归一化
			hash_combine(info.borderColor);
			hash_combine(info.unnormalizedCoordinates ? vk::True : vk::False);   // 格式化bool32

			return static_cast<size_t>(seed);
		}
	};

	// 自定义比较仿函数
	struct SamplerCreateInfoEqual {
		bool operator()(const vk::SamplerCreateInfo& a,
			const vk::SamplerCreateInfo& b) const noexcept {

			auto funcNormalizeBool32 = [](const vk::Bool32& b)->bool {
				return b != vk::False;
				};

			// 依次比较所有影响采样器创建的字段
			return a.magFilter == b.magFilter &&
				a.minFilter == b.minFilter &&
				a.mipmapMode == b.mipmapMode &&
				a.addressModeU == b.addressModeU &&
				a.addressModeV == b.addressModeV &&
				a.addressModeW == b.addressModeW &&
				a.mipLodBias == b.mipLodBias &&
				funcNormalizeBool32(a.anisotropyEnable) == funcNormalizeBool32(b.anisotropyEnable) &&
				a.maxAnisotropy == b.maxAnisotropy &&
				funcNormalizeBool32(a.compareEnable) == funcNormalizeBool32(b.compareEnable) &&
				a.compareOp == b.compareOp &&
				a.minLod == b.minLod &&
				a.maxLod == b.maxLod &&
				a.borderColor == b.borderColor &&
				funcNormalizeBool32(a.unnormalizedCoordinates) == funcNormalizeBool32(b.unnormalizedCoordinates);
			// 忽略 sType 和 pNext
		}
	};

	// 只管Get尽可能不要调用delete
	class SamplerManager {
	private:
		using SamplerMap = std::unordered_map<vk::SamplerCreateInfo, ImageSampler*, SamplerCreateInfoHash, SamplerCreateInfoEqual>;

		SamplerMap m_mapImageSampler;
		ImageSamplerID m_nIDCounter;
		// 最大支持的各向异性采样
		float m_fMaxAnis;

		SamplerManager();
		~SamplerManager();

		ImageSamplerID GenImageSamplerID();

	private:
		static SamplerManager* s_pInstance;

	public:
		static void Init();
		static void Release();

		static ImageSampler* GetDefaultImageSampler();
		static ImageSampler* GetImageSampler(const vk::SamplerCreateInfo& sci);
		// 尽可能不要调用
		// 可能产出野指针
		static void DeleteImageSampler(ImageSampler* );
	};


} // namespace LT