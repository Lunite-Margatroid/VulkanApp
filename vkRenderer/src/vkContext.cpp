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

		std::vector<const char* > layers;
#ifdef _DEBUG
		layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

		// Create vk instance
		vk::InstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.setFlags(vk::InstanceCreateFlags())
			.setPApplicationInfo(&appInfo)
			.setPEnabledLayerNames(layers)
			.setEnabledLayerCount(layers.size())
			// .setEnabledExtensionCount(static_cast<uint32_t>(extensiont));
			;

		try {
			m_vkInstance = vk::createInstance(instanceCreateInfo);
		}
		catch (const std::exception& e) {
			std::cout << "vkInstance Create Failed" << std::endl;
		}

		PickPhyDevice();
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

	void vkContext::PickPhyDevice() {
		auto phyDevices = m_vkInstance.enumeratePhysicalDevices();
		m_phyDevice = phyDevices[0];
		// 第一个图形设备
		// m_phyDevice.getFeatures(); // 支持的特性
		LOG_INFO("Physical Device: %s\n", m_phyDevice.getProperties().deviceName);
	}
}