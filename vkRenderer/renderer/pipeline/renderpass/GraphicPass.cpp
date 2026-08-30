// 编码utf-8
#include "vkRendererCommon.h"
#include "GraphicPass.hpp"
#include "vkContext.h"
#include "SlangCompiler.hpp"

#include "VertexBuffer.h"
#include "SwapChain.h"

#include "ImageManager.h"
#include "BufferManager.h"

namespace LT {
	GraphicPass::~GraphicPass()
	{
		vk::Device& device = vkContext::GetVkDevice();

		if (m_vkShaderModule)
		{
			device.destroyShaderModule(m_vkShaderModule);
		}

		if (m_vkPipeline)
		{
			device.destroyPipeline(m_vkPipeline);
		}

		if(m_vkDescSetLayout)
		{
			device.destroyDescriptorSetLayout(m_vkDescSetLayout);
		}

		if(m_vkPipelineLayout)
		{
			device.destroyPipelineLayout(m_vkPipelineLayout);
		}

	}
	void GraphicPass::Init() {

		vk::Device& device = vkContext::GetVkDevice();

		// 创建着色器
		{
			m_sShaderModuleInfo.Clear();
			std::vector<BYTE> binShader = SlangCompiler::GetInstance().CompileShader(m_vecShaderModuleSrc, GenGraphicPPMacroDesc(m_nFlag), m_sShaderModuleInfo);

			vk::ShaderModuleCreateInfo smci(
				{},
				static_cast<uint32_t>(binShader.size()),
				reinterpret_cast<uint32_t*>(binShader.data()),
				nullptr
			);

			m_vkShaderModule = device.createShaderModule(smci);
		}

		vk::PipelineShaderStageCreateInfo pssciVert = {};
		pssciVert
			.setModule(m_vkShaderModule)
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setPName("VertMain")
			;
		vk::PipelineShaderStageCreateInfo pssciFrag = {};
		pssciFrag
			.setModule(m_vkShaderModule)
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setPName("FragMain")
			;
		std::array<vk::PipelineShaderStageCreateInfo, 2> psscis = { pssciVert, pssciFrag };


		// 顶点状态
		std::vector<vk::VertexInputBindingDescription> vecInputBindDesc;
		std::vector<vk::VertexInputAttributeDescription> vecVertInputDesc;
		// 低11位是顶点输入标志
		VertexChannelFlag nVertexChannelFlag = static_cast<uint32_t>(m_nFlag & ((1 << 11) - 1));
		GenVertexAttributeDesc(nVertexChannelFlag, vecInputBindDesc, vecVertInputDesc);

		vk::PipelineVertexInputStateCreateInfo pvisci = {};
		pvisci
			.setVertexAttributeDescriptions(vecVertInputDesc)
			.setVertexBindingDescriptions(vecInputBindDesc)
			;

		// 顶点装配状态
		vk::PipelineInputAssemblyStateCreateInfo piasci = {};

		// 图元类型
		piasci.setTopology(GetPrimitiveTopology(m_nFlag));

		// 指定动态状态
		std::array<vk::DynamicState, 2> dss = {
			vk::DynamicState::eViewport , vk::DynamicState::eScissor
		};
		vk::PipelineDynamicStateCreateInfo pdsci = {};
		pdsci
			.setPDynamicStates(dss.data())
			.setDynamicStateCount(dss.size())
			;

		// Viewport
		vk::PipelineViewportStateCreateInfo pvsci = {};
		pvsci
			.setViewportCount(1)
			.setScissorCount(1)
			;

		// 光栅化状态
		vk::PipelineRasterizationStateCreateInfo prsci = {};
		prsci
			.setDepthClampEnable(vk::False)
			.setRasterizerDiscardEnable(vk::False)
			.setPolygonMode(GetPolygonMode(m_nFlag))
			.setCullMode(IsBackCull(m_nFlag) ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
			.setFrontFace(IsClockwiseFront(m_nFlag) ? vk::FrontFace::eClockwise : vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(vk::False)
			.setLineWidth(GetLineWidth(m_nFlag))
			;

		// 超采样
		vk::PipelineMultisampleStateCreateInfo pmsci = {};
		pmsci
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(vk::False)
			;

		// 混合状态
		vk::PipelineColorBlendAttachmentState pcbas = {};
		pcbas
			.setBlendEnable(IsBlendEnabled(m_nFlag) ? vk::True : vk::False)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			;
		vk::PipelineColorBlendStateCreateInfo pcbsci = {};
		pcbsci
			.setLogicOpEnable(vk::False)
			.setAttachmentCount(1)
			.setPAttachments(&pcbas)
			;

		// ------------- Pipeline Layout ------------------
		std::vector<vk::DescriptorSetLayoutBinding> layouts;
		// const buffer
		for (const auto& bindingInfo : m_sShaderModuleInfo.m_vecConstBufferBindingInfo) {
			
			vk::DescriptorSetLayoutBinding dslb;
			dslb
				.setBinding(bindingInfo.nIndex)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setStageFlags(bindingInfo.GetShaderStageFlag())
				;

			layouts.push_back(dslb);
		}

		// Texture2D
		for(const auto& BindingInfo : m_sShaderModuleInfo.m_vecTexture2DBindingInfo) {
			vk::DescriptorSetLayoutBinding dslb;
			dslb
				.setBinding(BindingInfo.nIndex)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setStageFlags(BindingInfo.GetShaderStageFlag())
				;

			layouts.push_back(dslb);
		}

		vk::DescriptorSetLayoutCreateInfo dslci;
		dslci.setBindings(layouts);
		
		m_vkDescSetLayout = device.createDescriptorSetLayout(dslci);

		vk::PipelineLayoutCreateInfo plci;
		plci
			.setSetLayoutCount(1)
			.setPSetLayouts(&m_vkDescSetLayout)
			.setPushConstantRanges(0)
			;

		m_vkPipelineLayout = device.createPipelineLayout(plci);

		// Allocate Descriptor Set
		std::vector<vk::DescriptorSetLayout> setlayouts(RENDERER_DEFAULT_FLIGHT_FRAME_NUM, m_vkDescSetLayout);
		vk::DescriptorSetAllocateInfo dsai;
		dsai
			.setDescriptorPool(vkContext::GetDescriptorPool())
			.setDescriptorSetCount(setlayouts.size())
			.setPSetLayouts(setlayouts.data())
			;

		m_vecDescriptorSets = device.allocateDescriptorSets(dsai);

		// 深度模板测试
		vk::PipelineDepthStencilStateCreateInfo pdssci = {};
		pdssci
			.setDepthTestEnable(vk::True)
			.setDepthWriteEnable(vk::True)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(vk::False)
			.setStencilTestEnable(vk::False)
			;

		vk::GraphicsPipelineCreateInfo gpci = {};
		gpci
			.setStages(psscis)
			.setPVertexInputState(&pvisci)
			.setPInputAssemblyState(&piasci)
			.setPViewportState(&pvsci)
			.setPRasterizationState(&prsci)
			.setPMultisampleState(&pmsci)
			.setPColorBlendState(&pcbsci)
			.setPDepthStencilState(&pdssci)
			.setPDynamicState(&pdsci)
			.setLayout(m_vkPipelineLayout)
			.setRenderPass(VK_NULL_HANDLE)
			.setPDepthStencilState(&pdssci)
			;
		
		std::array<vk::Format, 1> arrColorAttachmentFormats = { SWAPCHAIN_DEFAULT_PIXEL_FORMAT };
		vk::Format eDepthStencilFormat = vk::Format::eD32Sfloat;
		vk::PipelineRenderingCreateInfo prci = {};
		prci
			.setColorAttachmentCount(arrColorAttachmentFormats.size())
			.setPColorAttachmentFormats(arrColorAttachmentFormats.data())
			.setDepthAttachmentFormat(eDepthStencilFormat)
			;

		gpci.setPNext(&prci);

		auto result = device.createGraphicsPipeline(nullptr, gpci);
		RENDERER_ASSERT(result.result == vk::Result::eSuccess, "create graphics pipeline failed");

		m_vkPipeline = result.value;
	}

	void GraphicPass::AddShaderModule(const char* strShaderModule)
	{
		m_vecShaderModuleSrc.push_back(strShaderModule);
	}

	void GraphicPass::SetRenderPassFlag(RenderPassFlag nFlag)
	{
		m_nFlag = nFlag;
	}

	RenderPassFlag GraphicPass::GetRenderPassFlag() const
	{
		return m_nFlag;
	}

	void GraphicPass::GenVertexAttributeDesc(VertexChannelFlag nVertexChannelFlag, std::vector<vk::VertexInputBindingDescription>& vecInputBindDesc, std::vector<vk::VertexInputAttributeDescription>& vertDesc)
	{

		vecInputBindDesc.clear();
		vertDesc.clear();

		uint32_t nLocationCount = 0;
		uint32_t nBindingCount = 0;
		uint32_t nOffset = 0;

		vk::Format arrFormats[5] = {
			vk::Format::eUndefined, vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat, vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat 
		};

		for (uint32_t i = 0; i < VERTEX_CHANNEL_TOTAL_COUNT; i++)
		{
			if (nVertexChannelFlag & (1 << i))
			{
				vertDesc.emplace_back(nLocationCount, nBindingCount, arrFormats[VERTEX_DIMENSION[i]], nOffset);
				nOffset += (VERTEX_DIMENSION[i] * sizeof(float));
				nLocationCount += 1;
			}
		}

		uint32_t nStride = nOffset;
		vecInputBindDesc.emplace_back(0, nStride, vk::VertexInputRate::eVertex);
	}

	void GraphicPass::RecordCommand(const RecordCommandInfo& sRecordInfo)
	{
		vk::CommandBuffer& cmdBuffer = vkContext::GetCmdBuffer(sRecordInfo.nFlightFrameIndex);
		vk::Extent2D extent = { sRecordInfo.nWidth , sRecordInfo.nHeight};
		uint32_t nFlightFrameIndex = sRecordInfo.nFlightFrameIndex;
		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vkPipeline);
		vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f);
		cmdBuffer.setViewport(0, viewport);
		vk::Rect2D scissor({ 0, 0 }, extent);
		cmdBuffer.setScissor(0, scissor);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_vkPipelineLayout, 0, m_vecDescriptorSets[nFlightFrameIndex], nullptr);


		vk::CommandBufferBeginInfo cbbi = {};
		cmdBuffer.begin(cbbi);

		// 转换颜色缓冲
		TransitionImageLayoutInfo sColorTransitionInfo;
		sColorTransitionInfo.vkCommandBuffer = cmdBuffer;
		sColorTransitionInfo.nImageID = sRecordInfo.vecImageIDColor[0];
		sColorTransitionInfo.srcAccessFlag = vk::AccessFlagBits2::eNone;
		sColorTransitionInfo.dstAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
		sColorTransitionInfo.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		sColorTransitionInfo.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		sColorTransitionInfo.eOldLayout = vk::ImageLayout::eUndefined;
		sColorTransitionInfo.eNewLayout = vk::ImageLayout::eColorAttachmentOptimal;
		sColorTransitionInfo.eImageAspect = vk::ImageAspectFlagBits::eColor;
		RenderPass::RecordTransitionImageLayout(sColorTransitionInfo);

		// 深度缓冲
		TransitionImageLayoutInfo sDepthTransitionInfo;
		sDepthTransitionInfo.vkCommandBuffer = cmdBuffer;
		sDepthTransitionInfo.nImageID = sRecordInfo.nDepthStencilID;
		sDepthTransitionInfo.srcAccessFlag = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
		sDepthTransitionInfo.dstAccessFlag = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
		sDepthTransitionInfo.srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
		sDepthTransitionInfo.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
		sDepthTransitionInfo.eOldLayout = vk::ImageLayout::eUndefined;
		sDepthTransitionInfo.eNewLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		sDepthTransitionInfo.eImageAspect = vk::ImageAspectFlagBits::eDepth;
		RenderPass::RecordTransitionImageLayout(sDepthTransitionInfo);

		// 绑定渲染目标

		// 颜色
		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::RenderingAttachmentInfo rai;
		rai
			.setImageView(ImageManager::GetInstance().GetNativeDeviceImageView(sRecordInfo.vecImageIDColor[0]))
			.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setClearValue(clearColor)
			;

		// 深度
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
		vk::RenderingAttachmentInfo raiDepth;
		raiDepth
			.setImageView(ImageManager::GetInstance().GetNativeDeviceImageView(sRecordInfo.nDepthStencilID))
			.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setClearValue(clearDepth)
			;

		// 录入渲染命令
		vk::RenderingInfo ri;
		
		vk::Rect2D renderArea({ 0, 0 }, { sRecordInfo.nWidth, sRecordInfo.nHeight });
		ri
			.setRenderArea(renderArea)
			.setLayerCount(1)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&rai)
			.setPDepthAttachment(&raiDepth)
			;

		cmdBuffer.beginRendering(ri);

		// 绑定渲染管线
		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vkPipeline);

		// 绑定顶点缓冲
		std::vector<vk::Buffer> vecVertexBuffer;
		std::vector<vk::DeviceSize> vecVertexBufferOffset;
		vecVertexBuffer.reserve(sRecordInfo.vecVertexBufferID.size());
		vecVertexBufferOffset.reserve(sRecordInfo.vecVertexBufferID.size());
		for (BufferID nID : sRecordInfo.vecVertexBufferID)
		{
			vecVertexBuffer.push_back(BufferManager::GetNativeDeviceBuffer(nID));
			vecVertexBufferOffset.push_back(0);
		}
		cmdBuffer.bindVertexBuffers(0, vecVertexBuffer, vecVertexBufferOffset);

		// 绑定索引缓冲
		cmdBuffer.bindIndexBuffer(BufferManager::GetNativeDeviceBuffer(sRecordInfo.nIndexBufferID), 0, vk::IndexType::eUint32);

		// 绑定
		// const buffer
		// texture resource
		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			m_vkPipelineLayout,
			0,
			m_vecDescriptorSets[nFlightFrameIndex],
			VK_NULL_HANDLE
		);

		// 创建并绑定Viewport Scissor State
		vk::Viewport viewport;
		viewport
			.setX(0)
			.setY(0)
			.setWidth(sRecordInfo.nWidth)
			.setHeight(sRecordInfo.nHeight)
			.setMinDepth(0.f)
			.setMaxDepth(1.f)
			;

		// Scissor
		vk::Rect2D scissor;
		scissor
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(vk::Extent2D(sRecordInfo.nWidth, sRecordInfo.nHeight))
			;

		cmdBuffer.setViewport(0, viewport);
		cmdBuffer.setScissor(0, scissor);

		
		auto nIndexCount = BufferManager::GetIndexBuffer(sRecordInfo.nIndexBufferID)->GetIndexCount();
		cmdBuffer.drawIndexed(nIndexCount, 1, 0, 0, 0);

		cmdBuffer.endRendering();

		// 颜色缓冲转为交换缓冲
		if (sRecordInfo.vecImageIDColor[0] == SWAPCHAIN_IMAGE_ID)
		{
			TransitionImageLayoutInfo sTransInfoForPresent;
			sTransInfoForPresent.vkCommandBuffer = cmdBuffer;
			sTransInfoForPresent.nImageID = sRecordInfo.vecImageIDColor[0];
			sTransInfoForPresent.eOldLayout = vk::ImageLayout::eColorAttachmentOptimal;
			sTransInfoForPresent.eNewLayout = vk::ImageLayout::ePresentSrcKHR;
			sTransInfoForPresent.srcAccessFlag = vk::AccessFlagBits2::eColorAttachmentWrite;
			sTransInfoForPresent.dstAccessFlag = vk::AccessFlagBits2::eNone;
			sTransInfoForPresent.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
			sTransInfoForPresent.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
			sTransInfoForPresent.eImageAspect = vk::ImageAspectFlagBits::eColor;
		}

		cmdBuffer.end();

	}

	void GraphicPass::Execute() {
		vk::Device& device = vkContext::GetVkDevice();
		vk::SwapchainKHR& swapchain = vkContext::GetNativeSwapChain();


		device.acquireNextImageKHR(swapchain, UINT64_MAX);
	}

} // namespace LT
