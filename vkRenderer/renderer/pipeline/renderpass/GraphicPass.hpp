// 编码utf-8
#pragma once
#include "RenderPass.hpp"
#include "DeviceImage.h"

#include "SlangCompiler.hpp"

namespace LT {

	class SwapChain;


	struct RecordCommandInfo {
		std::vector<ImageID> vecImageIDColor;
		ImageID nDepthStencilID;
		std::vector<BufferID> vecVertexBufferID;
		BufferID nIndexBufferID;

		uint32_t nFlightFrameIndex;
		uint32_t nWidth;
		uint32_t nHeight;
	};


	class GraphicPass : public RenderPass
	{
	protected:
		std::vector<std::string> m_vecShaderModuleSrc;
		RenderPassFlag m_nFlag;
		vk::ShaderModule m_vkShaderModule;
		ShaderModuleInfo m_sShaderModuleInfo;
		vk::DescriptorSetLayout m_vkDescSetLayout;
		vk::PipelineLayout m_vkPipelineLayout;
		std::vector<vk::DescriptorSet> m_vecDescriptorSets;
		vk::Pipeline m_vkPipeline;

	public:
		~GraphicPass();


		void Init();

		void AddShaderModule(const char* strShaderModule);
		void SetRenderPassFlag(RenderPassFlag nFlag);
		RenderPassFlag GetRenderPassFlag() const;

		void RecordCommand(const RecordCommandInfo& sRecordInfo);

		void Execute() override;

	public:
		static void GenVertexAttributeDesc(VertexChannelFlag nVertexChannelFlag, std::vector<vk::VertexInputBindingDescription>& vecInputBindDesc, std::vector<vk::VertexInputAttributeDescription>& vertDesc);

	};
} // namespace LT