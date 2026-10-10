// 辅助函数

#include "vkRendererCommon.h"
#include "vkRendererUtil.hpp"

namespace LT {
	namespace util {
		vk::Instance CreateVulkanInstance(const char* const* extensions, uint32_t nCount) {
			vk::ApplicationInfo appInfo;
			appInfo.setPApplicationName("vkRenderer")
				.setApplicationVersion(1)
				.setPEngineName("Lunite")
				.setEngineVersion(1)
				.setApiVersion(VK_API_VERSION_1_4);

			std::vector<const char* > layers;


			// 验证层
#ifdef _DEBUG
			layers.push_back("VK_LAYER_KHRONOS_validation");
#endif


			std::vector<const char*> exts;
			for (int i = 0; i < nCount; i++)
			{
				exts.push_back(extensions[i]);
			}

			// Create vk instance
			vk::InstanceCreateInfo instanceCreateInfo;
			instanceCreateInfo.setFlags(vk::InstanceCreateFlags())
				.setPApplicationInfo(&appInfo)
				.setPEnabledLayerNames(layers)
				.setEnabledLayerCount(layers.size())
				.setPEnabledExtensionNames(exts)
				// .setEnabledExtensionCount(static_cast<uint32_t>(extensiont));
				;

			return vk::createInstance(instanceCreateInfo);
		}

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

				return {x, y, z};
			}
		}

	}
}

