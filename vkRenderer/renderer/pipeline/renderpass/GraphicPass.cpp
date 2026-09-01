// 编码utf-8
#include "vkRendererCommon.h"
#include "GraphicPass.hpp"
#include "vkContext.h"
#include "SlangCompiler.hpp"

#include "VertexBuffer.h"
#include "SwapChain.h"

#include "ImageManager.h"
#include "BufferManager.h"
#include "SamplerManager.h"

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

		for(auto & descSet : m_vecVkDescSetLayout)
		{
			device.destroyDescriptorSetLayout(descSet);
		}
		m_vecVkDescSetLayout.clear();


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
		{
			std::vector<vk::DescriptorSetLayoutBinding> bindingsVert;
			std::vector<vk::DescriptorSetLayoutBinding> bindingsFrag;
			std::vector<vk::DescriptorSetLayoutBinding> bindingsVertAndFrag;

			auto funcAddToBindingSet = [&](BindingSpace eSpace, const vk::DescriptorSetLayoutBinding& vkBindings) {
				switch (eSpace) {
					case BindingSpace::eVertexShader:
						bindingsVert.push_back(vkBindings);
						break;
					case BindingSpace::eFragmentShader:
						bindingsFrag.push_back(vkBindings);
						break;
					case BindingSpace::eVertAndFragShader:
						bindingsVertAndFrag.push_back(vkBindings);
						break;
					default:break;
				};
			};

			// const buffer
			for (const auto& bindingInfo : m_sShaderModuleInfo.m_vecConstBufferBindingInfo) {

				vk::DescriptorSetLayoutBinding dslb;
				dslb
					.setBinding(bindingInfo.nIndex)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setDescriptorCount(1)
					.setStageFlags(GetShaderStageFlag(bindingInfo.eSpace))
					;

				funcAddToBindingSet(bindingInfo.eSpace, dslb);
			}

			// Texture2D
			for (const auto& bindingInfo : m_sShaderModuleInfo.m_vecTexture2DBindingInfo) {
				vk::DescriptorSetLayoutBinding dslb;
				dslb
					.setBinding(bindingInfo.nIndex)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setDescriptorCount(1)
					.setStageFlags(GetShaderStageFlag(bindingInfo.eSpace))
					;
				funcAddToBindingSet(bindingInfo.eSpace, dslb);
			}


			vk::DescriptorSetLayoutCreateInfo dslciVert;
			dslciVert.setBindings(bindingsVert);
			vk::DescriptorSetLayoutCreateInfo dslciFrag;
			dslciFrag.setBindings(bindingsFrag);
			vk::DescriptorSetLayoutCreateInfo dslciVertAndFrag;
			dslciVertAndFrag.setBindings(bindingsVertAndFrag);

			m_vecVkDescSetLayout.resize(3);
			m_vecVkDescSetLayout[static_cast<size_t>(BindingSpace::eVertexShader)] = device.createDescriptorSetLayout(dslciVert);
			m_vecVkDescSetLayout[static_cast<size_t>(BindingSpace::eFragmentShader)] = device.createDescriptorSetLayout(dslciFrag);
			m_vecVkDescSetLayout[static_cast<size_t>(BindingSpace::eVertAndFragShader)] = device.createDescriptorSetLayout(dslciVertAndFrag);
		}
		vk::PipelineLayoutCreateInfo plci;
		plci
			.setSetLayoutCount(m_vecVkDescSetLayout.size())
			.setPSetLayouts(m_vecVkDescSetLayout.data())
			.setPushConstantRanges(0)
			;

		m_vkPipelineLayout = device.createPipelineLayout(plci);

		// Allocate Descriptor Set
		std::vector<vk::DescriptorSetLayout> setlayouts;
		setlayouts.insert(setlayouts.end(), m_vecVkDescSetLayout.begin(), m_vecVkDescSetLayout.end());
		vk::DescriptorSetAllocateInfo dsai;
		dsai
			.setDescriptorPool(vkContext::GetDescriptorPool())
			.setDescriptorSetCount(setlayouts.size())
			.setPSetLayouts(setlayouts.data())
			;

		// Frame0
		m_vecDescriptorSets0 = device.allocateDescriptorSets(dsai);
		// Frame1
		m_vecDescriptorSets1 = device.allocateDescriptorSets(dsai);

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
		std::vector<vk::DescriptorSet>* pDescSet = (nFlightFrameIndex == 0 ? &m_vecDescriptorSets0 : &m_vecDescriptorSets1);
		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			m_vkPipelineLayout,
			0,
			*pDescSet,
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
			RenderPass::RecordTransitionImageLayout(sTransInfoForPresent);
		}

		cmdBuffer.end();

	}

	void GraphicPass::Submit(const GraphicSubmitInfo& sSubmitInfo) {
		vk::Device& device = vkContext::GetVkDevice();
		vk::CommandBuffer& cmdBuffer = vkContext::GetCmdBuffer(sSubmitInfo.nFlightFrameIndex);

		// 提交信息
		vk::SubmitInfo si = {};
		si
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmdBuffer)
			;

		// 需要等待的信号量
		if (sSubmitInfo.vecSemToWait.size() > 0)
		{
			si.setWaitSemaphores(sSubmitInfo.vecSemToWait);
			si.setWaitDstStageMask(sSubmitInfo.vecSwapDstMask);
		}
		// 需要发射的信号量
		if (sSubmitInfo.vecSemToSignal.size() > 0)
		{
			si.setSignalSemaphores(sSubmitInfo.vecSemToSignal);
		}

		if (sSubmitInfo.vkFenceToSet)
		{
			vkContext::GetCmdQueue().submit(
				si,
				sSubmitInfo.vkFenceToSet // 需要设置的Fence
			);
		}
		else
		{
			vkContext::GetCmdQueue().submit(si);
		}
	}

	void GraphicPass::BindConstBuffer(BufferID id, BindingSpace eSpace, uint32_t nBindingIndex, uint32_t nFrameIndex)
	{
		Buffer* pBuffer = BufferManager::GetBuffer(id);

		vk::DescriptorBufferInfo dbi = {};
		dbi
			.setBuffer(pBuffer->GetNativeBuffer())
			.setOffset(0)
			.setRange(pBuffer->Size())
			;



		std::array<vk::WriteDescriptorSet, 1> wds = {};
		wds[0]
			.setDstBinding(nBindingIndex)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setPBufferInfo(&dbi)
			;

		std::vector<vk::DescriptorSet>* pVecDescSets = (nFrameIndex == 0 ? &m_vecDescriptorSets0 : &m_vecDescriptorSets1);
		
		wds[0].setDstSet((*pVecDescSets)[static_cast<size_t>(eSpace)]);


		vkContext::GetVkDevice().updateDescriptorSets(wds, {});

	}

	void GraphicPass::BindImage2D(ImageID id, BindingSpace eSpace, uint32_t nBindingIndex, uint32_t nFrameIndex)
	{
		vk::DescriptorImageInfo ddi = {};

		ddi
			.setImageView(ImageManager::GetNativeDeviceImageView(id))
			.setSampler(SamplerManager::GetDefaultImageSampler()->GetNativeSampler())
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			;

		std::array<vk::WriteDescriptorSet, 1> wds;
		wds[0]
			.setDstBinding(nBindingIndex)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImageInfo(&ddi)
			;

		std::vector<vk::DescriptorSet>* pVecDescSets = (nFrameIndex == 0 ? &m_vecDescriptorSets0 : &m_vecDescriptorSets1);

		wds[0].setDstSet((*pVecDescSets)[static_cast<size_t>(eSpace)]);
		vkContext::GetVkDevice().updateDescriptorSets(wds, {});
	}

} // namespace LT
