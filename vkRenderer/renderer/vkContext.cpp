#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"

#include "SlangComplier.h"
#include "Engine.h"

#include "DeviceMemoryManager.h"

namespace LT {
	vkContext* vkContext::s_pVkContext = nullptr;

	vkContext::vkContext(const std::vector<const char* >& extensions, HWND hWnd)
	{
		m_bIsAnisotropySampleSupported = false;

		CreateVkInstance(extensions);
		PickPhyDevice();

		CreateSurface(hWnd);
		CreateVkDevice();

		// 着色器编译器初始化
		SlangComplier::Init();

		CreateCommandPool();
		CreateCommandBuffer();

		CreateDescriptorPool();

	}

	vkContext::~vkContext() {


		m_vkDevice.destroyDescriptorPool(m_vkDescriptorPool);

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

		// 检查特性
		vkContext::CheckPhysicalDeivceFeatures();
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
		// VMA支持扩展
		if (DeviceMemoryManager::VK_KHR_get_memory_requirements2_enabled)
			extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_KHR_dedicated_allocation_enabled)
			extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_KHR_bind_memory2_enabled)
			extensions.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_EXT_memory_budget_enabled)
			extensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_AMD_device_coherent_memory_enabled)
			extensions.push_back(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_KHR_buffer_device_address_enabled && DeviceMemoryManager::GetVkVersion() < VK_API_VERSION_1_2)
			extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_EXT_memory_priority_enabled)
			extensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_KHR_maintenance5_enabled && DeviceMemoryManager::GetVkVersion() < VK_API_VERSION_1_4)
			extensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
		if (DeviceMemoryManager::VK_KHR_external_memory_win32_enabled)
			extensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);

		deviceCreateInfo
			.setEnabledExtensionCount(extensions.size())
			.setPEnabledExtensionNames(extensions);
		for (const char* extension : extensions)
		{
			LOG_INFO("Device extension: %s", extension);
		}


#define PnextChainPushFront(mainStruct, newStruct) \
do{\
(newStruct).pNext = (mainStruct).pNext;\
(mainStruct).pNext = &(newStruct);\
} while(false)

		// 设备特性
		vk::PhysicalDeviceFeatures2 pdf2 = {};
		pdf2.features.setSamplerAnisotropy(vk::True); // 各向异性采样
		pdf2.features.setSparseBinding(DeviceMemoryManager::g_SparseBindingEnabled ? VK_TRUE : VK_FALSE);


		vk::PhysicalDeviceVulkan11Features pdv11f = {};
		pdv11f.setShaderDrawParameters(vk::True);

		vk::PhysicalDeviceVulkan13Features pdv13f = {};
		pdv13f.setDynamicRendering(vk::True);	// 动态渲染
		pdv13f.setSynchronization2(vk::True);	// 异步信号

		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT pdedsfe = {};
		pdedsfe.setExtendedDynamicState(vk::True);	// 动态渲染扩展

		PnextChainPushFront(pdf2, pdv11f);
		PnextChainPushFront(pdv11f, pdv13f);
		PnextChainPushFront(pdv13f, pdedsfe);

		vk::PhysicalDeviceCoherentMemoryFeaturesAMD pdcmfAMD = {};
		if (DeviceMemoryManager::VK_AMD_device_coherent_memory_enabled)
		{
			pdcmfAMD.setDeviceCoherentMemory(vk::True);
			PnextChainPushFront(pdf2, pdcmfAMD);
		}

		vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR pdbdafKHR = {};
		if (DeviceMemoryManager::VK_KHR_buffer_device_address_enabled)
		{
			pdbdafKHR.setBufferDeviceAddress(vk::True);
			PnextChainPushFront(pdf2, pdbdafKHR);
		}

		vk::PhysicalDeviceMemoryPriorityFeaturesEXT pdmpfEXT = {};
		if (DeviceMemoryManager::VK_EXT_memory_priority_enabled)
		{
			pdmpfEXT.setMemoryPriority(vk::True);
			PnextChainPushFront(pdf2, pdmpfEXT);
		}

		vk::PhysicalDeviceMaintenance5FeaturesKHR pdm5fKHR = {};
		if (DeviceMemoryManager::VK_KHR_maintenance5_enabled)
		{
			pdm5fKHR.setMaintenance5(vk::True);
			PnextChainPushFront(pdf2, pdm5fKHR);
		}

		deviceCreateInfo.setPNext(&pdf2);
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

	void vkContext::CreateDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize,2> dps;
		dps[0]
			.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(RENDERER_DEFAULT_FLIGHT_FRAME_NUM)
			;

		dps[1]
			.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(RENDERER_DEFAULT_FLIGHT_FRAME_NUM)
			;

		vk::DescriptorPoolCreateInfo dpci;
		dpci
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setMaxSets(RENDERER_DEFAULT_FLIGHT_FRAME_NUM)
			.setPoolSizeCount(dps.size())
			.setPPoolSizes(&dps[0])
			;

		m_vkDescriptorPool = m_vkDevice.createDescriptorPool(dpci);
	}

	void vkContext::CheckPhysicalDeivceFeatures()
	{
		auto features = m_phyDevice.getFeatures2<
			vk::PhysicalDeviceFeatures2, 
			vk::PhysicalDeviceVulkan13Features, 
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

		// 各向异性采样
		m_bIsAnisotropySampleSupported = features.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;

		RENDERER_ASSERT(features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering, "The device did not support dynamic rendering.");
		RENDERER_ASSERT(features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2, "");
		RENDERER_ASSERT(features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState, "");

		DeviceMemoryManager::CheckVMASupportedExtension(m_phyDevice);
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

	vk::CommandPool& vkContext::GetCmdPool()
	{
		return GetInstance().m_vkCommandPool;
	}

	vk::CommandBuffer& vkContext::GetCmdBuffer(unsigned int nIndex)
	{
		return GetInstance().m_vecCommandBuffers[nIndex];
	}

	vk::DescriptorPool& vkContext::GetDescriptorPool()
	{
		return GetInstance().m_vkDescriptorPool;
	}

	vk::CommandBuffer vkContext::BeginSingleTimeCmdBuffer()
	{
		vk::Device& device = vkContext::GetVkDevice();
		vk::Queue& queue = vkContext::GetCmdQueue();

		// 创建临时的command buffer
		vk::CommandBufferAllocateInfo cbai;
		cbai
			.setCommandBufferCount(1)
			.setCommandPool(vkContext::GetCmdPool())
			.setLevel(vk::CommandBufferLevel::ePrimary)
			;

		std::vector<vk::CommandBuffer> vkTempCmdBuffers = device.allocateCommandBuffers(cbai);

		// 开始记录命令
		vk::CommandBufferBeginInfo cbbi;
		cbbi.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		vkTempCmdBuffers[0].begin(cbbi);

		return vkTempCmdBuffers[0];
	}

	void vkContext::EndSingleTimeCmdBuffer(vk::CommandBuffer& cmdBuffer)
	{
		vk::Queue& queue = vkContext::GetCmdQueue();
		vk::Device& device = vkContext::GetVkDevice();

		// 结束记录命令
		cmdBuffer.end();

		// 提交命令
		vk::SubmitInfo si;
		si.setCommandBuffers(cmdBuffer);
		queue.submit(si);

		// 等待命令执行完成
		queue.waitIdle();

		device.freeCommandBuffers(vkContext::GetCmdPool(), cmdBuffer);
	}

	void vkContext::TransitionImageLayout(
		vk::CommandBuffer& cmdBuffer,
		const vk::Image& image,
		vk::PipelineStageFlags srcStage, 
		vk::PipelineStageFlags dstStage,
		vk::ImageLayout srcLayout,
		vk::ImageLayout dstLayout
	)
	{


		vk::ImageSubresourceRange isr;
		isr
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setLayerCount(1)
			.setLevelCount(1)
			;

		std::array<vk::ImageMemoryBarrier, 1> imb;
		imb[0]
			.setOldLayout(srcLayout)
			.setNewLayout(dstLayout)
			.setImage(image)
			.setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
			.setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
			.setSubresourceRange(isr)
			;

		std::vector<vk::MemoryBarrier> mb;
		std::vector<vk::BufferMemoryBarrier> bmb;

		cmdBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits::eByRegion, mb, bmb, imb);
	}


	vk::Device& vkContext::GetVkDevice() {
		return GetInstance().m_vkDevice;
	}

	vk::Instance& vkContext::GetVulkanInstance()
	{
		return GetInstance().m_vkInstance;
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

	bool vkContext::GetIsAnisotropySampleSupported()
	{
		return GetInstance().m_bIsAnisotropySampleSupported;
	}

} // namespace LT