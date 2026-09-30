// 编码utf-8
#pragma once
#include "RenderPass.hpp"
#include "DeviceImage.h"

#include "SlangCompiler.hpp"

namespace LT {

	class SwapChain;


	struct RecordCommandInfo {
		FlightFrameIndex nFlightFrameIndex;
		uint32_t nWidth;
		uint32_t nHeight;


		BufferID nIndexBufferID;
		ImageID nDepthStencilID;
		std::vector<ImageID> vecImageIDColor;
		std::vector<BufferID> vecVertexBufferID;
	};


	struct GraphicSubmitInfo {
	
		FlightFrameIndex nFlightFrameIndex;

		std::vector<vk::Semaphore> vecSemToWait;
		// 等待信号量的Mask
		// size和vecSemToWait一致
		std::vector<vk::PipelineStageFlags> vecSemWaitMasks;
		std::vector<vk::Semaphore> vecSemToSignal;
		vk::Fence vkFenceToSet;

		GraphicSubmitInfo()
			:nFlightFrameIndex(0)
		{}
	};

	struct SlotKey {
		BindingSpace eSpace; 
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
		std::vector<std::pair<std::string, std::string>> m_vecShaderCode;
		RenderPassFlag m_nFlag;
		vk::ShaderModule m_vkShaderModule;
		ShaderModuleInfo m_sShaderModuleInfo;
		// index和BindingSpace一致
		// 0 vert
		// 1 frag
		// 2 vert and frag
		std::vector<vk::DescriptorSetLayout> m_vecVkDescSetLayout;

		//std::vector<vk::DescriptorSet> m_vecDescriptorSets0;
		//std::vector<vk::DescriptorSet> m_vecDescriptorSets1;

		vk::Pipeline m_vkPipeline;

		std::unordered_map<SlotKey, int64_t, SlotKeyHash> m_mapSlots;

	public:
		GraphicPass();
		~GraphicPass();

		GraphicPass& operator = (const GraphicPass&) = delete;
		GraphicPass& operator = (GraphicPass&&) = delete;
		GraphicPass(GraphicPass&&) = delete;
		GraphicPass(const GraphicPass&) = delete;

		void Init(vk::PipelineLayout vkPipelineLayout);

		void AddShaderModule(const char* strShaderModule);
		void AddShaderModule(const std::string& strName, const std::string& strCode);
		void AddShaderModule(std::string&& strName, std::string&& strCode);
		void SetRenderPassFlag(RenderPassFlag nFlag);
		RenderPassFlag GetRenderPassFlag() const;

		void RecordCommand(const RecordCommandInfo& sRecordInfo);

		void Submit(const GraphicSubmitInfo& sSubmitInfo);

		void BindConstBuffer(ConstBufferHandle nConstBufferHandle, BindingSpace eSpace, uint32_t nBindingIndex);
		void BindImage2D(ImageID id, BindingSpace eSpace, uint32_t eBindingIndex);
		void BindResourceToDevice(FlightFrameIndex nFlightFrameIndex);

	public:
		static void GenVertexAttributeDesc(VertexChannelFlag nVertexChannelFlag, std::vector<vk::VertexInputBindingDescription>& vecInputBindDesc, std::vector<vk::VertexInputAttributeDescription>& vertDesc);

	};
} // namespace LT