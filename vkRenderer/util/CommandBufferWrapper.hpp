#pragma once
#include "FenceManager.hpp"
namespace LT {
	struct CmdSubmitInfo {
		std::vector<vk::Semaphore> vecSemToWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMasks;
		std::vector<vk::Semaphore> vecSemToSignal;
	};

	class CommandBufferWrapper {
		struct CmdBufferWithFence {
			vk::CommandBuffer cmdBuffer;
			FenceRef refFence;

			CmdBufferWithFence() 
				: cmdBuffer(VK_NULL_HANDLE)
				, refFence(INVALID_FENCE_HANDLE)
			{}

			CmdBufferWithFence(const vk::CommandBuffer& cmdBuf, const FenceRef fence)
				: cmdBuffer(cmdBuf)
				, refFence(fence)
			{}
		};

	private:
		std::array<CmdBufferWithFence, RENDERER_COMMAND_BUFFER_COUNT> m_arrCmdBuffers;
		std::queue<CmdBufferWithFence*> m_qCmdBuffers;
		vk::Queue m_queue;
	public:
		CommandBufferWrapper();
		CommandBufferWrapper(const CommandBufferWrapper&) = delete;
		CommandBufferWrapper(CommandBufferWrapper&&) = default;
		CommandBufferWrapper& operator=(const CommandBufferWrapper&) = delete;
		CommandBufferWrapper& operator=(CommandBufferWrapper&&) = default;

		void Init(vk::Device device, vk::Queue cmdQueue, vk::CommandPool vkCmdPool);
		void Destroy(vk::Device device, vk::CommandPool vkCmdPool);
		FenceRef Submit(const CmdSubmitInfo& sSubmitInfo);

		void PipelineBarrier2(const vk::DependencyInfo sDependencyInfo);
		void Begin(const vk::CommandBufferBeginInfo& sBeginInfo);
		void BeginRendering(const vk::RenderingInfo& sRenderingInfo);
		void BindPipeline(const vk::PipelineBindPoint& pipelineBindPoint, const vk::Pipeline pipeline);
		void BindVertexBuffers(uint32_t firstBinding, const std::vector<vk::Buffer> buffers, const std::vector<vk::DeviceSize> & offsets);
		void BindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType);
		void BindDescriptorSets(
			vk::PipelineBindPoint pipelineBindPoint,
			const vk::PipelineLayout& layout,
			uint32_t firstSet,
			const std::vector<vk::DescriptorSet>& descriptorSets,
			const std::vector<uint32_t>& dynamicOffsets);

		void SetViewport(uint32_t firstViewport,const std::vector<vk::Viewport> & viewports);
		void SetScissor(uint32_t firstScissor, const std::vector<vk::Rect2D> & scissors);
		void DrawIndexed(
			uint32_t         indexCount,
			uint32_t         instanceCount,
			uint32_t         firstIndex,
			int32_t          vertexOffset,
			uint32_t         firstInstance);

		void EndRendering();
		void End();
	};

} // namspace LT
