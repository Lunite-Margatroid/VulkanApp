#include "vkRendererCommon.h"
#include "vkContext.h"

namespace LT {
	vkContext* vkContext::s_pVkContext = nullptr;

	vkContext::vkContext(const std::vector<const char* >& extensions, HWND hWnd) {
		CreateVkInstance(extensions);
		PickPhyDevice();

		CreateSurface(hWnd);
		CreateVkDevice();
	}

	vkContext::~vkContext() {
		m_vkDevice.destroy();

		m_vkInstance.destroySurfaceKHR(m_vkSurface);
		m_vkInstance.destroy();
	}

	vkContext& vkContext::GetInstance() {
		assert(s_pVkContext);
		return *s_pVkContext;
	}

	void vkContext::Release() {
		if (s_pVkContext)
			delete s_pVkContext;
		s_pVkContext = nullptr;
	}

	void vkContext::Init(const std::vector<const char* >& extensions, HWND hWnd) {
		if (!s_pVkContext)
		{
			s_pVkContext = new vkContext(extensions, hWnd);
		}
	}


	void vkContext::CreateVkInstance(const std::vector<const char* >& extensions) {
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

		// Create vk instance
		vk::InstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.setFlags(vk::InstanceCreateFlags())
			.setPApplicationInfo(&appInfo)
			.setPEnabledLayerNames(layers)
			.setEnabledLayerCount(layers.size())
			.setPEnabledExtensionNames(extensions)
			// .setEnabledExtensionCount(static_cast<uint32_t>(extensiont));
			;

		try {
			m_vkInstance = vk::createInstance(instanceCreateInfo);
		}
		catch (const std::exception& e) {
			LOG_ERROR("vkInstance Create Failed.\n");
		}

		for (const char* extension : extensions)
		{
			LOG_INFO("set extension: %s\n", extension);
		}
	}

	void vkContext::PickPhyDevice() {
		auto phyDevices = m_vkInstance.enumeratePhysicalDevices();
		m_phyDevice = phyDevices[0];
		// 第一个图形设备
		// m_phyDevice.getFeatures(); // 支持的特性
		LOG_INFO("Physical Device: %s\n", static_cast<const char*>(m_phyDevice.getProperties().deviceName.data()));
	}

	void vkContext::CreateVkDevice() {
		// 创建命令队列和设备

		// 获取队列族 找到支持图形的队列族？
		auto vecQueueFamilys = m_phyDevice.getQueueFamilyProperties();
		for (int i = 0; i < vecQueueFamilys.size(); i++)
		{
			const auto& queueFamilyProperty = vecQueueFamilys[i];
			if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
				m_nQueueFamilyIndex = i;
				break;
			}
		}

		// 查询支持surface的队列族

		if (m_vkSurface)
		{
			for (int i = 0; i < vecQueueFamilys.size(); i++)
			{
				const auto& queueFamilyProperty = vecQueueFamilys[i];
				if (m_phyDevice.getSurfaceSupportKHR(i, m_vkSurface)) {
					m_nQueueIndexForSurface = i;
					break;
				}
			}

		}
		else
		{
			LOG_WARNING("Surface is not being created.(It is OK if only off-screen rendering is used.)\n");
		}



		// 默认的 图形的命令队列
		vk::DeviceQueueCreateInfo queueCreateInfo;
		float priority = 1.0f;
		queueCreateInfo
			.setPQueuePriorities(&priority)
			.setQueueCount(1)
			.setQueueFamilyIndex(m_nQueueFamilyIndex.value());



		// 创建设备
		vk::DeviceCreateInfo deviceCreateInfo;
		if (IsGraphicsSurfaceSameQueue() || !m_nQueueIndexForSurface.has_value())
		{
			// if 存在一个同时支持Surface和Graphics的命令队列 or 不需要创建Surface
			// then 只需要创建一个命令队列
			deviceCreateInfo
				.setQueueCreateInfos(queueCreateInfo)
				.setQueueCreateInfoCount(1);
		}
		else
		{
			vk::DeviceQueueCreateInfo queueCreateInfoForSurface;
			queueCreateInfoForSurface
				.setPQueuePriorities(&priority)
				.setQueueCount(1)
				.setQueueFamilyIndex(m_nQueueIndexForSurface.value());

			std::array queueCreateInfos = {queueCreateInfo, queueCreateInfoForSurface};

			deviceCreateInfo
				.setQueueCreateInfos(queueCreateInfos)
				.setQueueCreateInfoCount(2);
		}

		// 设备扩展
		// 交换链扩展
		std::vector<const char*> extensions{ vk::KHRSwapchainExtensionName };
		deviceCreateInfo
			.setEnabledExtensionCount(extensions.size())
			.setPEnabledExtensionNames(extensions);
		for (const char* extension : extensions)
		{
			LOG_INFO("Device extension: %s\n", extension);
		}


		m_vkDevice = m_phyDevice.createDevice(deviceCreateInfo);

		m_vkQueue = m_vkDevice.getQueue(m_nQueueFamilyIndex.value(), 0);
		if (!IsGraphicsSurfaceSameQueue() && m_nQueueIndexForSurface.has_value())
		{
			m_vkQueueForSurface = m_vkDevice.getQueue(m_nQueueIndexForSurface.value(), 0);
		}
	}

	void vkContext::CreateSurface(HWND hWnd) {
		if (!hWnd)
			return;
		
		vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo
			.setHwnd(hWnd)
			.setPNext(nullptr)
			.setHinstance(GetModuleHandle(nullptr));

		m_vkSurface = m_vkInstance.createWin32SurfaceKHR(surfaceCreateInfo);
	}

	inline bool vkContext::IsGraphicsSurfaceSameQueue() const noexcept {
		return m_nQueueFamilyIndex.has_value() && \
			m_nQueueIndexForSurface.has_value() && \
			m_nQueueFamilyIndex.value() == m_nQueueIndexForSurface.value();
	}

	inline vk::Queue vkContext::GetCmdQueue() noexcept {
		return m_vkQueue;
	}

	inline vk::Queue vkContext::GetCmdQueueForSurface() noexcept {
		if (IsGraphicsSurfaceSameQueue()) {
			return GetCmdQueue();
		}
		else {
			return m_vkQueueForSurface;
		}
	}
}