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
	}
}

