#include "RandomGenerator.hpp"

namespace LT
{
	namespace util
	{
		namespace RandomGenerator
		{
			std::random_device g_RandomDevice;
			std::mt19937 g_RandomGen(g_RandomDevice());

			std::array<float, 3> RandomSampleSphere(float fRadius)
			{
				float theta = UniformDistribution(0.f, 3.1415926f * 2);
				float z = UniformDistribution(-1.f, 1.f);
				float t = std::sqrtf(1 - z * z) * fRadius;

				float x = t * std::cosf(theta);
				float y = t * std::sinf(theta);
				z *= fRadius;

				return { x, y, z };
			}
		}
	}
}