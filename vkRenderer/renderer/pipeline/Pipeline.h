#pragma once


namespace LT {
	class Pipeline {
	protected:
		// debug shader
		vk::ShaderModule m_vkShaderMod;

		vk::PipelineLayout m_vkPipelineLayout;
		vk::Pipeline m_vkPipeline;

		std::vector<vk::Semaphore> m_vkSemRenderFinish;	// 数量与swapchain的image一致
		std::vector<vk::Semaphore> m_vkSemPresentComplete;	// 数量与flight frame一致

		std::vector<vk::Fence> m_vkFenceDraw;	// 数量与flight frame一致

		uint64_t m_nFrameCount;
	
		// 该实例不由Pipeline管理
		vk::Buffer m_vkVertexBuffer;

		void CreateSyncObjects();
		void RecordCommandBufferDebug(unsigned int imageIndex, unsigned int nFrameIndex);

		void TransitionImageLayout(
			uint32_t nImageIndex,
			uint32_t nFrameIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessFlag,
			vk::AccessFlags2 dstAccessFlag,
			vk::PipelineStageFlags2 srcStageFlag,
			vk::PipelineStageFlags2 dstStageFlag
		);

	public:
		Pipeline();
		~Pipeline();
		void DrawFrame();

		vk::Pipeline& GetNativePipeline();

		void SetVertexBuffer(vk::Buffer vkVertexBuffer);
	};
}// namespace LT