// 编码utf-8
#include "vkRendererCommon.h"
#include "GraphicPass.hpp"
#include "vkContext.h"
#include "SlangCompiler.hpp"

#include "VertexBuffer.h"


namespace LT {
	GraphicPass::~GraphicPass()
	{
		vk::Device& device = vkContext::GetVkDevice();

		if (m_vkShaderModule)
		{
			device.destroyShaderModule(m_vkShaderModule);
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

		SLANG_TEXTURE_2D;

		// Pipeline Layout
		std::vector<vk::DescriptorSetLayoutBinding> layouts;
		// const buffer
		//for (uint32_t nBindingIndex : m_sShaderModuleInfo.m_vecConstBufferBindingIndex) {
			//layouts.emplace_back({
			//		
			//	});
		//}


		
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

} // namespace LT
