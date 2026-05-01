#include "vkRendererCommon.h"
#include "vkContext.h"

namespace LT {
	vkContext* vkContext::s_pVkContext = nullptr;

	vkContext::vkContext() {

		vk::ApplicationInfo appInfo;
		appInfo.setPApplicationName("vkRenderer")
			.setApplicationVersion(1)
			.setPEngineName("Lunite")
			.setEngineVersion(1)
			.setApiVersion(VK_API_VERSION_1_4);

		// Create vk instance
		vk::InstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.setFlags(vk::InstanceCreateFlags())
			.setPApplicationInfo(&appInfo)
			// .setEnabledExtensionCount(static_cast<uint32_t>(extensiont));
			;

		try {
			m_vkInstance = vk::createInstance(instanceCreateInfo);
		}
		catch (const std::exception& e) {
			std::cout << "vkInstance Create Failed" << std::endl;
		}
	}

	vkContext::~vkContext() {
		m_vkInstance.destroy();
	}

	vkContext& vkContext::GetInstance() {
		if (!s_pVkContext)
			s_pVkContext = new vkContext();
		return *s_pVkContext;
	}

	void vkContext::Release() {
		if (s_pVkContext)
			delete s_pVkContext;
		s_pVkContext = nullptr;
	}

	void vkContext::Init() {
		if (!s_pVkContext)
		{
			s_pVkContext = new vkContext();
		}
	}
}