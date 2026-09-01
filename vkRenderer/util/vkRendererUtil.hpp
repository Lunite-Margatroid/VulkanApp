// 辅助函数
#pragma once

namespace LT
{
	namespace util
	{
		vk::Instance CreateVulkanInstance(const char* const* extensions, uint32_t nCount);

		template<typename T, typename MaskType>
		inline void SetBit(T& nFlag, MaskType mask) {
			nFlag |= (mask);
		}

		template<typename T, typename MaskType>
		inline void ResetBit(T& nFlag, MaskType mask) {
			nFlag &= (~(mask));
		}

		template<typename T, typename MaskType>
		inline void SetBit(T& nFlag, MaskType nMask, T nValue) {
			nFlag = (nFlag & ~nMask) | (nValue & nMask);
		}

	} // namespace util
}// namespace LT