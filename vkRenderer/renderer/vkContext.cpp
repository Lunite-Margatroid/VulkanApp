#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"

#include "SlangComplier.h"
#include "Engine.h"
namespace LT {
	vkContext* vkContext::s_pVkContext = nullptr;

	vkContext::vkContext(const std::vector<const char* >& extensions, HWND hWnd)
	{
		CreateVkInstance(extensions);
		PickPhyDevice();

		CreateSurface(hWnd);
		CreateVkDevice();

		// 着色器编译器初始化
		SlangComplier::Init();

		CreateCommandPool();
		CreateCommandBuffer();


	}

	vkContext::~vkContext() {
		// 销毁Command Pool
		m_vkDevice.destroyCommandPool(m_vkCommandPool);
		// command buffer会跟随command pool 自动释放
		m_vecCommandBuffers.clear();


		m_pSwapChain.reset();

		m_vkDevice.destroy();

		m_vkInstance.destroySurfaceKHR(m_vkSurface);
		m_vkInstance.destroy();



		SlangComplier::Release();
	}

	vkContext& vkContext::GetInstance() {
		RENDERER_ASSERT(s_pVkContext, "vkContext Did not init.");
		return *s_pVkContext;
	}

	void vkContext::InitSwapChain()
	{
		if (!(GetInstance().m_pSwapChain))
		{
			GetInstance().m_pSwapChain.reset(new SwapChain());
		}
		else
		{
			LOG_WARNING("Repeating Init Swap Chain.");
		}
	}



	void vkContext::ReleaseSwapChain()
	{
		if (GetInstance().m_pSwapChain)
		{
			GetInstance().m_pSwapChain.reset();
		}
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

		LOG_INFO("Create Vk Instance.\n");

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

		for (const char* layer : layers)
		{
			LOG_INFO("Enabled layers: %s\n", layer);
		}

		for (const char* extension : extensions)
		{
			LOG_INFO("Enabled extensions: %s\n", extension);
		}

		LOG_INFO("Create Vk Instance End.\n");
	}

	void vkContext::PickPhyDevice() {
		// 获取所有的图形设备
		auto phyDevices = m_vkInstance.enumeratePhysicalDevices();
		for (int i = 0; i < phyDevices.size(); i++)
		{
			LOG_INFO("PhysicalDevice[%d]:%s\n", i, static_cast<const char*>(phyDevices[i].getProperties().deviceName.data()));
		}


		m_phyDevice = phyDevices[0];
		// 第一个图形设备
		// m_phyDevice.getFeatures(); // 支持的特性
		LOG_INFO("Pick Physical Device: %s\n", static_cast<const char*>(m_phyDevice.getProperties().deviceName.data()));
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
			// 如果支持Graphics和支持Surface的指令队列不是同一个命令队列
			// 则创建两个命令队列
			vk::DeviceQueueCreateInfo queueCreateInfoForSurface;
			queueCreateInfoForSurface
				.setPQueuePriorities(&priority)
				.setQueueCount(1)
				.setQueueFamilyIndex(m_nQueueIndexForSurface.value());

			std::array queueCreateInfos = { queueCreateInfo, queueCreateInfoForSurface };

			deviceCreateInfo
				.setQueueCreateInfos(queueCreateInfos)
				.setQueueCreateInfoCount(2);
		}

		// 设备扩展
		// 支持交换链
		std::vector<const char*> extensions{ vk::KHRSwapchainExtensionName };
		extensions.push_back(vk::KHRShaderDrawParametersExtensionName);
		extensions.push_back(vk::KHRDynamicRenderingExtensionName);
		deviceCreateInfo
			.setEnabledExtensionCount(extensions.size())
			.setPEnabledExtensionNames(extensions);
		for (const char* extension : extensions)
		{
			LOG_INFO("Device extension: %s", extension);
		}

		// 设备特性
		vk::PhysicalDeviceFeatures2 pdf2 = {};
		vk::PhysicalDeviceVulkan11Features pdv11f;
		pdv11f.setShaderDrawParameters(vk::True);
		vk::PhysicalDeviceVulkan13Features pdv13f;
		pdv13f.setDynamicRendering(vk::True);
		pdv13f.setSynchronization2(vk::True);
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT pdedsfe;
		pdedsfe.setExtendedDynamicState(vk::True);
		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan11Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
			pdf2,
			pdv11f,
			pdv13f,
			pdedsfe
		};

		deviceCreateInfo.setPNext(&(featureChain.get<vk::PhysicalDeviceFeatures2>()));


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

	void vkContext::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo cpci;
		cpci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(m_nQueueFamilyIndex.value());

		m_vkCommandPool = m_vkDevice.createCommandPool(cpci);
	}

	void vkContext::CreateCommandBuffer()
	{
		RENDERER_ASSERT(m_vkCommandPool, "Command Pool did not Init.");

		vk::CommandBufferAllocateInfo cbai;
		cbai.setCommandPool(m_vkCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(RENDERER_DEFAULT_FLIGHT_FRAME_NUM);	// 数量跟flight frame一致

		m_vecCommandBuffers = m_vkDevice.allocateCommandBuffers(cbai);

	}


	void vkContext::ResizeSwapChain(unsigned int width, unsigned int height)
	{
		vkContext& context = GetInstance();

		if (context.m_pSwapChain->m_sSwapChainInfo.height == height && context.m_pSwapChain->m_sSwapChainInfo.width)
		{
			return;
		}

		vkContext::WaitIdel();
		vkContext::ReleaseSwapChain();
		vkContext::InitSwapChain();
	}



	vk::Queue& vkContext::GetCmdQueue() {
		return GetInstance().m_vkQueue;
	}

	vk::Queue& vkContext::GetCmdQueueForSurface() {
		if (GetInstance().IsGraphicsSurfaceSameQueue()) {
			return GetCmdQueue();
		}
		else {
			return GetInstance().m_vkQueueForSurface;
		}
	}

	vk::CommandBuffer& vkContext::GetCmdBuffer(unsigned int nIndex)
	{
		return GetInstance().m_vecCommandBuffers[nIndex];
	}


	vk::Device& vkContext::GetVkDevice() {
		return GetInstance().m_vkDevice;
	}

	vk::PhysicalDevice& vkContext::GetPhysicalDevice()
	{
		return GetInstance().m_phyDevice;
	}

	vk::SwapchainKHR& vkContext::GetNativeSwapChain()
	{
		// TODO: 在此处插入 return 语句
		return GetInstance().m_pSwapChain->NativeVKSwapChain();
	}

	SwapChain& vkContext::GetSwapChain() {
		return *(GetInstance().m_pSwapChain);
	}

} // namespace LT