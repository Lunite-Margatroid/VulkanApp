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
			std::vector<BYTE> binShader = SlangCompiler::GetInstance().ComplieShader(m_vecShaderModuleSrc, GenGraphicPPMacroDesc(m_nFlag));

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
