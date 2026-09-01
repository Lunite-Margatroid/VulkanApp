// 编码utf-8
#pragma once
#include "RenderPass.hpp"
#include "DeviceImage.h"

#include "SlangCompiler.hpp"

namespace LT {

	class SwapChain;


	struct RecordCommandInfo {
		uint32_t nFlightFrameIndex;
		uint32_t nWidth;
		uint32_t nHeight;


		BufferID nIndexBufferID;
		ImageID nDepthStencilID;
		std::vector<ImageID> vecImageIDColor;
		std::vector<BufferID> vecVertexBufferID;
	};


	struct GraphicSubmitInfo {
	
		uint32_t nFlightFrameIndex;

		std::vector<vk::Semaphore> vecSemToWait;
		std::vector<vk::PipelineStageFlags> vecSwapDstMask;
		std::vector<vk::Semaphore> vecSemToSignal;
		vk::Fence vkFenceToSet;

		GraphicSubmitInfo()
		{}
	};

	class GraphicPass : public RenderPass
	{
	protected:
		std::vector<std::string> m_vecShaderModuleSrc;
		RenderPassFlag m_nFlag;
		vk::ShaderModule m_vkShaderModule;
		ShaderModuleInfo m_sShaderModuleInfo;
		// index和BindingSpace一致
		// 0 vert
		// 1 frag
		// 2 vert and frag
		std::vector<vk::DescriptorSetLayout> m_vecVkDescSetLayout;
		vk::PipelineLayout m_vkPipelineLayout;

		std::vector<vk::DescriptorSet> m_vecDescriptorSets0;
		std::vector<vk::DescriptorSet> m_vecDescriptorSets1;

		vk::Pipeline m_vkPipeline;

	public:
		GraphicPass() = default;
		~GraphicPass();


		void Init();

		void AddShaderModule(const char* strShaderModule);
		void SetRenderPassFlag(RenderPassFlag nFlag);
		RenderPassFlag GetRenderPassFlag() const;

		void RecordCommand(const RecordCommandInfo& sRecordInfo);

		void Submit(const GraphicSubmitInfo& sSubmitInfo);

		void BindConstBuffer(BufferID id, BindingSpace eSpace, uint32_t nBindingIndex, uint32_t nFrameIndex);
		void BindImage2D(ImageID id, BindingSpace eSpace, uint32_t eBindingIndex, uint32_t nFrameIndex);

	public:
		static void GenVertexAttributeDesc(VertexChannelFlag nVertexChannelFlag, std::vector<vk::VertexInputBindingDescription>& vecInputBindDesc, std::vector<vk::VertexInputAttributeDescription>& vertDesc);

	};
} // namespace LT