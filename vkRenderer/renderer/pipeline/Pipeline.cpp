#include "vkRendererCommon.h"
#include "vkContext.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "SlangComplier.h"
namespace LT {
	Pipeline::Pipeline() {

		CreateDebugSyncObjects();

		// 创建着色器
		std::vector<std::string> vecEntryPoint = { "VertMain", "FragMain" };
		std::filesystem::path shaderPath = "./slang/HelloTriangleShader.slang";
		auto buffer = SlangComplier::GetInstance().CompileFromFile(shaderPath, vecEntryPoint);

		vk::ShaderModuleCreateInfo smci(
			{},
			static_cast<uint32_t>(buffer.size()),
			reinterpret_cast<uint32_t*>(buffer.data()),
			nullptr
		);

		m_vkShaderMod = vkContext::GetVkDevice().createShaderModule(smci);

		// 顶点输入状态
		vk::PipelineVertexInputStateCreateInfo pvisci;
		std::vector<vk::VertexInputBindingDescription> bindingDesc;
		std::vector<vk::VertexInputAttributeDescription> vertDesc;
		vertDesc.emplace_back(0, 0, vk::Format::eR32G32Sfloat, 0);
		vertDesc.emplace_back(1, 0, vk::Format::eR32G32B32Sfloat, 8);

		bindingDesc.emplace_back(0, 20, vk::VertexInputRate::eVertex);
		pvisci.setVertexAttributeDescriptions(vertDesc)
			.setVertexBindingDescriptions(bindingDesc);

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
		m_vkPipelineLayout = vkContext::GetVkDevice().createPipelineLayout(plci);


		vk::PipelineShaderStageCreateInfo pssciVert;
		pssciVert.setModule(m_vkShaderMod);
		pssciVert.setStage(vk::ShaderStageFlagBits::eVertex);
		pssciVert.setPName("VertMain");

		vk::PipelineShaderStageCreateInfo pssciFrag;
		pssciFrag.setModule(m_vkShaderMod);
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
		gpci.setLayout(m_vkPipelineLayout);
		gpci.setRenderPass(VK_NULL_HANDLE);


		vk::PipelineRenderingCreateInfo prci;
		prci
			.setColorAttachmentCount(1)
			.setPColorAttachmentFormats(&SWAPCHAIN_DEFAULT_PIXEL_FORMAT)
			;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pcic = {
			gpci, prci
		};

		auto result = vkContext::GetVkDevice().createGraphicsPipeline(nullptr, pcic.get<vk::GraphicsPipelineCreateInfo>());

		RENDERER_ASSERT(result.has_value(), "Pipeline Create Failed.");

		m_vkPipeline = result.value;
	}

	void Pipeline::CreateSyncObjects() {
		m_vkSemRenderFinish.resize(SWAPCHAIN_DEFAULT_IMAGE_NUM);
		m_vkSemPresentComplete.resize(RENDERER_DEFAULT_FLIGHT_FRAME_NUM);
		m_vkFenceDraw.resize(RENDERER_DEFAULT_FLIGHT_FRAME_NUM);

		for (int i = 0; i < m_vkSemRenderFinish.size(); i++)
		{
			m_vkSemRenderFinish[i] = vkContext::GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
		}

		for (int i = 0; i < m_vkSemPresentComplete.size(); i++)
		{
			m_vkSemPresentComplete[i] = vkContext::GetVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
		}

		for (int i = 0; i < m_vkFenceDraw.size(); i++)
		{
			m_vkFenceDraw[i] = vkContext::GetVkDevice().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		}
	}


	void Pipeline::RecordCommandBufferDebug(unsigned int imageIndex, unsigned int nFrameIndex)
	{
		auto debugCommandBuffer = vkContext::GetCmdBuffer(nFrameIndex);

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
			.setImageView(vkContext::GetSwapChain().m_imageViews[imageIndex])
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
					{static_cast<unsigned int>(vkContext::GetSwapChain().m_sSwapChainInfo.width), static_cast<unsigned int>(vkContext::GetSwapChain().m_sSwapChainInfo.height)}
				})
			.setLayerCount(1)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&rai)
			;

		debugCommandBuffer.beginRendering(ri);


		// 绑定图形管线
		debugCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vkPipeline);


		// Viewport 和 Scissor 被指定为动态状态
		// 创建并绑定

		// Viewport
		vk::Viewport viewport;
		viewport
			.setX(0)
			.setY(0)
			.setWidth(static_cast<float>(vkContext::GetSwapChain().m_sSwapChainInfo.width))
			.setHeight(static_cast<float>(vkContext::GetSwapChain().m_sSwapChainInfo.height))
			.setMinDepth(0.f)
			.setMaxDepth(1.0f);

		// scissor
		vk::Rect2D scissor;
		scissor.setOffset(vk::Offset2D(0, 0));
		scissor.setExtent(vk::Extent2D(vkContext::GetSwapChain().m_sSwapChainInfo.width, vkContext::GetSwapChain().m_sSwapChainInfo.height));

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

	void Pipeline::TransitionImageLayout(
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
			.setImage(vkContext::GetSwapChain().m_sSwapChainInfo.images[nImageIndex])
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

		vkContext::GetCmdBuffer(nFrameIndex).pipelineBarrier2(di);
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


	void Pipeline::DrawFrame() {
		
	}
} //namespace LT