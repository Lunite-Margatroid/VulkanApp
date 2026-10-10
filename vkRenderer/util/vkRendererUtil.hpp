// 辅助函数
#pragma once
#include <random>

namespace LT
{
	namespace util
	{
		vk::Instance CreateVulkanInstance(const char* const* extensions, uint32_t nCount);

		// -------------- math -----------------
		namespace RandomGenerator
		{
			extern std::random_device g_RandomDevice;
			extern std::mt19937 g_RandomGen;
			template<typename T>
			T UniformDistribution(T a, T b) {
				if constexpr (std::is_floating_point_v<T>)
				{
					// 浮点数 [a, b)
					std::uniform_real_distribution<T> dist(a, b);
					return dist(g_RandomGen);
				}
				else
				{
					// 整数 [a, b]
					std::uniform_int_distribution<T> dist(a, b);
					return dist(g_RandomGen);
				}
			}

			std::array<float, 3> RandomSampleSphere(float fRadius = 1.f);
		}

	} // namespace util

}// namespace LT