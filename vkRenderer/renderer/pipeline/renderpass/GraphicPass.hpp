// 编码utf-8
#pragma once
#include "RenderPass.hpp"
#include "DeviceImage.h"

#include "SlangCompiler.hpp"

#include "DescriptorSetWrapper.hpp"

#include "FenceManager.hpp"

namespace LT {

	class SwapChain;


	struct RecordCommandInfo {
		FlightFrameIndex nFlightFrameIndex;
		uint32_t nWidth;
		uint32_t nHeight;

		ImageID nDepthStencilID;
		std::vector<ImageID> vecImageIDColor;
		std::vector<std::vector<BufferID>> vecVertexBufferID;
		std::vector<BufferID> vecIndexBufferID;


		vk::AttachmentLoadOp eColorLoadOp;
		vk::AttachmentLoadOp eDepthLoadOp;

		RecordCommandInfo() 
			: nFlightFrameIndex(0)
			, nWidth(0)
			, nHeight(0)
			, nDepthStencilID(INVALID_IMAGE_ID)
			, eColorLoadOp(vk::AttachmentLoadOp::eLoad)
			, eDepthLoadOp(vk::AttachmentLoadOp::eLoad)
		{}
	};


	struct GraphicSubmitInfo {
	
		FlightFrameIndex nFlightFrameIndex;

		std::vector<vk::Semaphore> vecSemToWait;
		// 等待信号量的Mask
		// size和vecSemToWait一致
		std::vector<vk::PipelineStageFlags> vecSemWaitMasks;
		std::vector<vk::Semaphore> vecSemToSignal;
		FenceRef refFence;

		GraphicSubmitInfo()
			: nFlightFrameIndex(0)
			, refFence(INVALID_FENCE_HANDLE)
		{}
	};

	struct SlotKey {
		ShaderStage eSpace; 
		uint32_t nBindingIndex;
		vk::DescriptorType eType;

		bool operator == (const SlotKey& other) const {
			return eSpace == other.eSpace && nBindingIndex == other.nBindingIndex && eType == other.eType;
		}
	};

	struct SlotKeyHash {
		size_t operator() (const SlotKey& key) const {
			size_t nHash = (static_cast<uint64_t>(key.eType) << 32) | (static_cast<uint64_t>(key.nBindingIndex) << 16) | (static_cast<uint64_t>(key.eSpace));
			return std::hash<uint64_t>{}(nHash);
		}
	};

	class GraphicPass : public RenderPass
	{
	protected:
		std::vector<std::string> m_vecShaderModuleSrc;
		ShaderSrc m_sShaderSrc;
		RenderPassFlag m_nFlag;
		// 材质注册的属性列表 用于生成MTL_PROP_宏
		std::vector<MtlProp> m_vecMtlProps;
		vk::ShaderModule m_vkShaderModule;
		ShaderModuleInfo m_sShaderModuleInfo;

		// 临时 不由GraphicPass管理
		std::vector<DescriptorSetWrapper*> m_vecDescriptorSets;

		vk::PipelineLayout m_vkPipelineLayout;

		vk::Pipeline m_vkPipeline;

		std::unordered_map<SlotKey, int64_t, SlotKeyHash> m_mapSlots;

	public:
		GraphicPass();
		~GraphicPass();

		GraphicPass& operator = (const GraphicPass&) = delete;
		GraphicPass& operator = (GraphicPass&&) = delete;
		GraphicPass(GraphicPass&&) = delete;
		GraphicPass(const GraphicPass&) = delete;

		bool Init(
			const std::vector<vk::DescriptorSetLayout>& vecVkDescSetLayout,
			bool bCreateModelMatPushConst = true
			);

		void SetDescriptorSets(const std::vector<DescriptorSetWrapper*>& vecDescriptorSets);
		/// <summary>
		/// 添加基于代码文件的模块
		/// </summary>
		/// <param name="strShaderModule"> 文件名 </param>
		/// <param name="vecDepMod">依赖的模块</param>
		void AddShaderModule(const char* strShaderModule, const std::vector<std::string>& vecDepMod);
		/// <summary>
		/// 添加基于源代码的模块
		/// </summary>
		/// <param name="strName"> 模块名 </param>
		/// <param name="strCode"> slang代码 </param>
		/// <param name="vecDepMod"> 依赖模块 </param>
		void AddShaderModule(const std::string& strName, const std::string& strCode, const std::vector<std::string>& vecDepMod);
		void SetRenderPassFlag(RenderPassFlag nFlag);
		RenderPassFlag GetRenderPassFlag() const;

		void SetMtlProps(const std::vector<MtlProp>& vecMtlProps);

		void RecordCommand(const RecordCommandInfo& sRecordInfo);

		void Submit(GraphicSubmitInfo& sInOutSubmitInfo);

		void Bind(CommandBufferWrapper* pCmdBuffer);

		vk::PipelineLayout GetPipelineLayout() const;

	public:
		static void GenVertexAttributeDesc(VertexChannelFlag nVertexChannelFlag, std::vector<vk::VertexInputBindingDescription>& vecInputBindDesc, std::vector<vk::VertexInputAttributeDescription>& vertDesc);

	};
} // namespace LT