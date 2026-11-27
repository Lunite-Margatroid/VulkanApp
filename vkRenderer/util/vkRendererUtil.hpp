// 辅助函数
#pragma once

#include "RandomGenerator.hpp"

namespace LT
{
	namespace util
	{
		vk::Instance CreateVulkanInstance(const char* const* extensions, uint32_t nCount);

		template <typename T>
		constexpr T AlignUp(T value, size_t alignment) {
			return (value + alignment - 1) & ~(alignment - 1);
		}
	} // namespace util

}// namespace LT