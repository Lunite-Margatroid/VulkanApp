#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"

#include "SlangComplier.h"
namespace LT {
	vkContext* vkContext::s_pVkContext = nullptr;

	vkContext::vkContext(const std::vector<const char* >& extensions, HWND hWnd)
		:m_nFrameCount(0)
	{
		CreateVkInstance(extensions);
		PickPhyDevice();

		CreateSurface(hWnd);
		CreateVkDevice();

		// 着色器编译器初始化
		SlangComplier::Init();

		CreateDebugShaderModule();

		CreateDebugPipeline();

		CreateCommandPool();
		CreateCommandBuffer();

		CreateDebugSyncObjects();

	}

	vkContext::~vkContext() {
		// 销毁异步对象
		for (int i = 0; i < m_vkSemRenderFinish.size(); i++)
		{
			m_vkDevice.destroySemaphore(m_vkSemRenderFinish[i]);
		}

		for (int i = 0; i < m_vkSemPresentComplete.size(); i++)
		{
			m_vkDevice.destroySemaphore(m_vkSemPresentComplete[i]);
		}

		for (int i = 0; i < m_vkFenceDraw.size(); i++)
		{
			m_vkDevice.destroyFence(m_vkFenceDraw[i]);
		}


		// 销毁Command Pool
		m_vkDevice.destroyCommandPool(m_vkCommandPool);
		// command buffer会跟随command pool 自动释放
		m_vecCommandBuffers.clear();


		// 销毁debug pipeline
		m_vkDevice.destroyPipeline(m_vkDebugPipeline);

		// 销毁debug shader module
		m_vkDevice.destroyShaderModule(m_vkDebugShaderMod);

		m_vkDevice.destroyPipelineLayout(m_vkDebugPipelineLayout);


		m_pSwapChain.reset();

		m_vkDevice.destroy();

		m_vkInstance.destroySurfaceKHR(m_vkSurface);
		m_vkInstance.destroy();



		SlangComplier::Release();
	}

	vkContext& vkContext::GetInstance() noexcept {
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

	void vkContext::DebugFrame()
	{
		GetInstance().DrawFrameDebug();
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

	void vkContext::CreateDebugSyncObjects()
	{
		m_vkSemRenderFinish.resize(SWAPCHAIN_DEFAULT_IMAGE_NUM);
		m_vkSemPresentComplete.resize(RENDERER_DEFAULT_FLIGHT_FRAME_NUM);
		m_vkFenceDraw.resize(RENDERER_DEFAULT_FLIGHT_FRAME_NUM);

		for (int i = 0; i < m_vkSemRenderFinish.size(); i++)
		{
			m_vkSemRenderFinish[i] = m_vkDevice.createSemaphore(vk::SemaphoreCreateInfo());
		}

		for (int i = 0; i < m_vkSemPresentComplete.size(); i++)
		{
			m_vkSemPresentComplete[i] = m_vkDevice.createSemaphore(vk::SemaphoreCreateInfo());
		}

		for (int i = 0; i < m_vkFenceDraw.size(); i++)
		{
			m_vkFenceDraw[i] = m_vkDevice.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		}
	}

	void vkContext::RecordCommandBufferDebug(unsigned int imageIndex, unsigned int nFrameIndex)
	{
		auto debugCommandBuffer = m_vecCommandBuffers[nFrameIndex];

		// 开始录入
		vk::CommandBufferBeginInfo cbbi;
		debugCommandBuffer.begin(cbbi);
		// 录入图像转换
		TransitionImageLayout(
			imageIndex,
			nFrameIndex,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput
		);

		vk::ClearValue clearValue = vk::ClearColorValue(0.f, 0.f, 0.f, 1.f);

		// 录入读写渲染目标操作
		vk::RenderingAttachmentInfo rai;
		rai
			.setImageView(m_pSwapChain->m_imageViews[imageIndex])
			.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setClearValue(clearValue)
			;

		// 录入渲染操作
		vk::RenderingInfo ri;
		ri
			.setRenderArea(
				{
					{ 0,0 },
					{static_cast<unsigned int>(m_pSwapChain->m_sSwapChainInfo.width), static_cast<unsigned int>(m_pSwapChain->m_sSwapChainInfo.height)}
				})
			.setLayerCount(1)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&rai)
			;

		debugCommandBuffer.beginRendering(ri);


		// 绑定图形管线
		debugCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vkDebugPipeline);


		// Viewport 和 Scissor 被指定为动态状态
		// 创建并绑定

		// Viewport
		vk::Viewport viewport;
		viewport
			.setX(0)
			.setY(0)
			.setWidth(static_cast<float>(m_pSwapChain->m_sSwapChainInfo.width))
			.setHeight(static_cast<float>(m_pSwapChain->m_sSwapChainInfo.height))
			.setMinDepth(0.f)
			.setMaxDepth(1.0f);

		// scissor
		vk::Rect2D scissor;
		scissor.setOffset(vk::Offset2D(0, 0));
		scissor.setExtent(vk::Extent2D(m_pSwapChain->m_sSwapChainInfo.width, m_pSwapChain->m_sSwapChainInfo.height));

		debugCommandBuffer.setViewport(0, viewport);
		debugCommandBuffer.setScissor(0, scissor);

		debugCommandBuffer.draw(3, 1, 0, 0);

		debugCommandBuffer.endRendering();

		TransitionImageLayout(
			imageIndex,
			nFrameIndex,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			{},
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eBottomOfPipe
		);

		debugCommandBuffer.end();
	}

	void vkContext::TransitionImageLayout(
		uint32_t nImageIndex,
		uint32_t nFrameIndex,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessFlag,
		vk::AccessFlags2 dstAccessFlag,
		vk::PipelineStageFlags2 srcStageFlag,
		vk::PipelineStageFlags2 dstStageFlag)
	{
		vk::ImageMemoryBarrier2 imageBarrier;
		imageBarrier
			.setSrcAccessMask(srcAccessFlag)
			.setSrcStageMask(srcStageFlag)
			.setDstAccessMask(dstAccessFlag)
			.setDstStageMask(dstStageFlag)
			.setOldLayout(oldLayout)
			.setNewLayout(newLayout)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(m_pSwapChain->m_sSwapChainInfo.images[nImageIndex])
			.setSubresourceRange(
				vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,	// base mipmap level
					1,	// level count
					0,	// base array layer
					1	// layer count
				)
			)
			;

		vk::DependencyInfo di;
		di.setDependencyFlags({})
			.setImageMemoryBarrierCount(1)
			.setPImageMemoryBarriers(&imageBarrier)
			;

		m_vecCommandBuffers[nFrameIndex].pipelineBarrier2(di);
	}

	void vkContext::WaitIdel()
	{
		vk::Queue& queueGraphics = GetInstance().GetCmdQueue();
		vk::Queue& queueSurface = GetInstance().GetCmdQueueForSurface();

		if (queueGraphics)
		{
			queueGraphics.waitIdle();
		}
		if (queueSurface) 
		{
			queueSurface.waitIdle();
		}
	}

	void vkContext::ResizeSwapChain(unsigned int width, unsigned int height)
	{
		vkContext & context = GetInstance();

		if (context.m_pSwapChain->m_sSwapChainInfo.height == height && context.m_pSwapChain->m_sSwapChainInfo.width)
		{
			return;
		}

		vkContext::WaitIdel();
		vkContext::ReleaseSwapChain();
		vkContext::InitSwapChain();
	}

	inline bool vkContext::IsGraphicsSurfaceSameQueue() const noexcept {
		return m_nQueueFamilyIndex.has_value() && \
			m_nQueueIndexForSurface.has_value() && \
			m_nQueueFamilyIndex.value() == m_nQueueIndexForSurface.value();
	}

	inline vk::Queue& vkContext::GetCmdQueue() noexcept {
		return m_vkQueue;
	}

	inline vk::Queue& vkContext::GetCmdQueueForSurface() noexcept {
		if (IsGraphicsSurfaceSameQueue()) {
			return GetCmdQueue();
		}
		else {
			return m_vkQueueForSurface;
		}
	}

	void vkContext::DrawFrameDebug()
	{
		if (m_pSwapChain->m_sSwapChainInfo.width <= 0 || m_pSwapChain->m_sSwapChainInfo.height <= 0)
			return;


		m_nFrameCount++;

		uint64_t nFrameIndex = m_nFrameCount % RENDERER_DEFAULT_FLIGHT_FRAME_NUM;
		uint64_t nLastFrameIndex = (m_nFrameCount - 1) % RENDERER_DEFAULT_FLIGHT_FRAME_NUM;
		// 等待上一帧绘制完成
		vk::Result result = m_vkDevice.waitForFences(m_vkFenceDraw[nFrameIndex], vk::True, UINT64_MAX);

		RENDERER_ASSERT(result == vk::Result::eSuccess, "Failed to wait fence.");

		m_vkDevice.resetFences(m_vkFenceDraw[nFrameIndex]);

		// 获取渲染缓冲
		// 等待交换链交换缓冲完成
		auto imageIndex = m_vkDevice.acquireNextImageKHR(
			GetSwapChain(),
			UINT64_MAX,
			m_vkSemPresentComplete[nFrameIndex] // 完成后发射信号
		);

		RENDERER_ASSERT(imageIndex.has_value(), "Acquire Image Failed.");

		unsigned int nImgIndex = imageIndex.value;

		// 录入渲染命令
		RecordCommandBufferDebug(nImgIndex, nFrameIndex);

		// 提交渲染命令
		vk::PipelineStageFlags flagWaitDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		vk::Semaphore* pSemRenderFinish = nullptr;
		if (nImgIndex == 0)
		{
			pSemRenderFinish = &m_vkSemRenderFinish[nImgIndex];
		}
		else if (nImgIndex == 1)
		{
			pSemRenderFinish = &m_vkSemRenderFinish[nImgIndex];
		}
		else
		{
			pSemRenderFinish = &m_vkSemRenderFinish[0];
		}


		vk::SubmitInfo si;
		si.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&m_vkSemPresentComplete[nFrameIndex]) // 等待交换链交换完成
			.setPWaitDstStageMask(&flagWaitDstStageMask)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&m_vecCommandBuffers[nFrameIndex])
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(pSemRenderFinish)	// 完成后发出信号
			;
		GetCmdQueue().submit(
			si,
			m_vkFenceDraw[nFrameIndex] // 渲染完成之前 禁止获取缓冲
		);

		// 交换链命令
		vk::PresentInfoKHR pi;
		pi.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(pSemRenderFinish)	// 等待渲染完成
			.setSwapchainCount(1)
			.setPSwapchains(&GetSwapChain())
			.setPImageIndices(&nImgIndex)
			;
		// 提交交换链命令
		vk::Result resultPresent = GetCmdQueueForSurface().presentKHR(pi);

		if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR)
		{
			WaitIdel();
			ReleaseSwapChain();
			InitSwapChain();
		}
		else
		{
			RENDERER_ASSERT(resultPresent == vk::Result::eSuccess, "Present Failed.");
		}



	}

	vk::Device& vkContext::GetVkDevice() noexcept {
		return GetInstance().m_vkDevice;
	}

	vk::SwapchainKHR& vkContext::GetSwapChain() noexcept
	{
		// TODO: 在此处插入 return 语句
		return GetInstance().m_pSwapChain->NativeVKSwapChain();
	}

	void vkContext::CreateDebugShaderModule() {
		std::vector<std::string> vecEntryPoint = { "VertMain", "FragMain" };
		std::filesystem::path shaderPath = "./slang/HelloTriangleShader.slang";
		auto buffer = SlangComplier::GetInstance().CompileFromFile(shaderPath, vecEntryPoint);

		vk::ShaderModuleCreateInfo smci(
			{},
			static_cast<uint32_t>(buffer.size()),
			reinterpret_cast<uint32_t*>(buffer.data()),
			nullptr
		);

		m_vkDebugShaderMod = m_vkDevice.createShaderModule(smci);


	}

	void vkContext::CreateDebugPipeline()
	{
		// 顶点着色器 输入状态
		vk::PipelineVertexInputStateCreateInfo pvisci;


		// 顶点装配状态
		vk::PipelineInputAssemblyStateCreateInfo piasci;
		// 图元类型
		// vk::PrimitiveTopology
		piasci.setTopology(vk::PrimitiveTopology::eTriangleList); // 三角形列表

		// 指定动态状态
		std::array<vk::DynamicState, 2> dss = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo pdsci;
		pdsci.setPDynamicStates(dss.data());
		pdsci.setDynamicStateCount(dss.size());
		// Viewport 创建信息
		vk::PipelineViewportStateCreateInfo pvsci;
		pvsci.setViewportCount(1);
		pvsci.setScissorCount(1);
		// 由于动态状态Viewport Scissor
		// Viewport 和 Scissor在Pipeline运行中可以动态改变
		// 不需要指定 Viewport 和 Scissor
		//pvsci.setPViewports(&viewport);
		//pvsci.setPScissors(&scissor);

		// 光栅化状态
		vk::PipelineRasterizationStateCreateInfo prsci;
		prsci
			.setDepthClampEnable(vk::False)
			.setRasterizerDiscardEnable(vk::False)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(vk::False)
			.setLineWidth(1.f)
			;

		// 超采样
		vk::PipelineMultisampleStateCreateInfo pmsci;
		pmsci
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(vk::False)
			;

		// 混合状态
		vk::PipelineColorBlendAttachmentState pcbas;
		pcbas
			.setBlendEnable(vk::True)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			;

		vk::PipelineColorBlendStateCreateInfo pcbsci;
		pcbsci
			.setLogicOpEnable(vk::False)
			.setAttachmentCount(1)
			.setPAttachments(&pcbas)
			;

		// Pipeline Layout
		vk::PipelineLayoutCreateInfo plci;
		plci
			.setSetLayoutCount(0)
			.setPushConstantRangeCount(0)
			;
		m_vkDebugPipelineLayout = m_vkDevice.createPipelineLayout(plci);


		vk::PipelineShaderStageCreateInfo pssciVert;
		pssciVert.setModule(m_vkDebugShaderMod);
		pssciVert.setStage(vk::ShaderStageFlagBits::eVertex);
		pssciVert.setPName("VertMain");

		vk::PipelineShaderStageCreateInfo pssciFrag;
		pssciFrag.setModule(m_vkDebugShaderMod);
		pssciFrag.setStage(vk::ShaderStageFlagBits::eFragment);
		pssciFrag.setPName("FragMain");

		std::array<vk::PipelineShaderStageCreateInfo, 2> stages = { pssciVert, pssciFrag };

		vk::GraphicsPipelineCreateInfo gpci;
		gpci.setStageCount(stages.size());
		gpci.setPStages(stages.data());
		gpci.setPVertexInputState(&pvisci);
		gpci.setPInputAssemblyState(&piasci);
		gpci.setPViewportState(&pvsci);
		gpci.setPRasterizationState(&prsci);
		gpci.setPMultisampleState(&pmsci);
		gpci.setPColorBlendState(&pcbsci);
		gpci.setPDynamicState(&pdsci);
		gpci.setLayout(m_vkDebugPipelineLayout);
		gpci.setRenderPass(VK_NULL_HANDLE);


		vk::PipelineRenderingCreateInfo prci;
		prci
			.setColorAttachmentCount(1)
			.setPColorAttachmentFormats(&SWAPCHAIN_DEFAULT_PIXEL_FORMAT)
			;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pcic = {
			gpci, prci
		};

		auto result = m_vkDevice.createGraphicsPipeline(nullptr, pcic.get<vk::GraphicsPipelineCreateInfo>());

		RENDERER_ASSERT(result.has_value(), "Pipeline Create Failed.");

		m_vkDebugPipeline = result.value;

	}

} // namespace LT