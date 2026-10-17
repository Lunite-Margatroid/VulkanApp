#include "vkRendererCommon.h"
#include "CommandBufferWrapper.hpp"
#include "vkContext.h"

namespace LT {
	CommandBufferWrapper::CommandBufferWrapper()
	{
	}
	void CommandBufferWrapper::Init(vk::Device device, vk::Queue cmdQueue, vk::CommandPool vkCmdPool)
	{
		m_queue = cmdQueue;

		vk::CommandBufferAllocateInfo cbai = {};
		cbai
			.setCommandBufferCount(RENDERER_COMMAND_BUFFER_COUNT)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(vkCmdPool)
			;
		std::vector<vk::CommandBuffer> cmdBuffers = device.allocateCommandBuffers(cbai);

		for (int i = 0; i < RENDERER_COMMAND_BUFFER_COUNT; ++i)
		{
			m_arrCmdBuffers[i] = CmdBufferWithFence(cmdBuffers[i], FenceRef(INVALID_FENCE_HANDLE));
			m_qCmdBuffers.push(i);
		}
	}
	void CommandBufferWrapper::Destroy(vk::Device device, vk::CommandPool vkCmdPool)
	{
		std::vector<vk::CommandBuffer> cmdBuffers;
		for(int i =0;i < m_arrCmdBuffers.size();++i)
		{
			cmdBuffers.push_back(m_arrCmdBuffers[i].cmdBuffer);
			m_arrCmdBuffers[i] = CmdBufferWithFence();
		}
		while(!m_qCmdBuffers.empty())
		{
			m_qCmdBuffers.pop();
		}

		device.freeCommandBuffers(vkCmdPool, cmdBuffers);
	}
	FenceRef CommandBufferWrapper::Submit(const CmdSubmitInfo& sSubmitInfo)
	{
		size_t index = m_qCmdBuffers.front();
		CmdBufferWithFence& cmdBufferWithFence = Front();

		vk::SubmitInfo si = {};
		si
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmdBufferWithFence.cmdBuffer)
			;

		// 需要等待的Semaphore
		if (sSubmitInfo.vecSemToWait.size() > 0)
		{
			si.setWaitSemaphores(sSubmitInfo.vecSemToWait);
			si.setWaitDstStageMask(sSubmitInfo.vecSemWaitMasks);
		}
		// 需要发射的Semaphore
		if (sSubmitInfo.vecSemToSignal.size() > 0)
		{
			si.setSignalSemaphores(sSubmitInfo.vecSemToSignal);
		}



		m_queue.submit(
			si,
			cmdBufferWithFence.refFence.GetNativeFence()
		);

		m_qCmdBuffers.pop();
		m_qCmdBuffers.push(index);

		return cmdBufferWithFence.refFence;
	}
	void CommandBufferWrapper::PipelineBarrier2(const vk::DependencyInfo sDependencyInfo)
	{
		Front().cmdBuffer.pipelineBarrier2(sDependencyInfo);
	}
	void CommandBufferWrapper::Begin(const vk::CommandBufferBeginInfo& sBeginInfo)
	{
		// 等待上一次提交的Fence
		Front().refFence.WaitFence();
		Front().refFence = FenceManager::CreateFence();

		Front().cmdBuffer.begin(sBeginInfo);
	}
	void CommandBufferWrapper::BeginRendering(const vk::RenderingInfo& sRenderingInfo)
	{
		Front().cmdBuffer.beginRendering(sRenderingInfo);
	}
	void CommandBufferWrapper::BindPipeline(const vk::PipelineBindPoint& pipelineBindPoint, const vk::Pipeline pipeline)
	{
		Front().cmdBuffer.bindPipeline(pipelineBindPoint, pipeline);
	}
	void CommandBufferWrapper::BindVertexBuffers(uint32_t firstBinding, const std::vector<vk::Buffer> buffers, const std::vector<vk::DeviceSize>& offsets)
	{
		Front().cmdBuffer.bindVertexBuffers(firstBinding, buffers, offsets);
	}
	void CommandBufferWrapper::BindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType)
	{
		Front().cmdBuffer.bindIndexBuffer(buffer, offset, indexType);
	}
	void CommandBufferWrapper::BindDescriptorSets(
		vk::PipelineBindPoint pipelineBindPoint,
		const vk::PipelineLayout& layout,
		uint32_t firstSet,
		const std::vector<DescriptorSetWrapper*>& descriptorSets,
		const std::vector<uint32_t>& dynamicOffsets)
	{
		std::vector<vk::DescriptorSet> vecDescriptorSets;
		for (DescriptorSetWrapper* pDescriptorSet : descriptorSets)
		{
			vecDescriptorSets.push_back(pDescriptorSet->GetNativeDescriptorSetWithoutWaiting());
			pDescriptorSet->SetFenceDrawing(Front().refFence);
			pDescriptorSet->PopFrontAndPushBack();
		}

		Front().cmdBuffer.bindDescriptorSets(pipelineBindPoint, layout, firstSet, vecDescriptorSets, dynamicOffsets);
	}
	void CommandBufferWrapper::SetViewport(uint32_t firstViewport, const std::vector<vk::Viewport>& viewports)
	{
		Front().cmdBuffer.setViewport(firstViewport, viewports);
	}
	void CommandBufferWrapper::SetScissor(uint32_t firstScissor, const std::vector<vk::Rect2D>& scissors)
	{
		Front().cmdBuffer.setScissor(firstScissor, scissors);
	}
	void CommandBufferWrapper::DrawIndexed(
		uint32_t         indexCount,
		uint32_t         instanceCount,
		uint32_t         firstIndex,
		int32_t          vertexOffset,
		uint32_t         firstInstance)
	{
		Front().cmdBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	void CommandBufferWrapper::EndRendering()
	{
		Front().cmdBuffer.endRendering();
	}
	void CommandBufferWrapper::End()
	{
		Front().cmdBuffer.end();
	}
	void CommandBufferWrapper::PushConstants(
		vk::PipelineLayout                       layout,
		vk::ShaderStageFlags                     stageFlags,
		uint32_t                             offset,
		uint32_t size,
		const void* pValues
	)
	{
		Front().cmdBuffer.pushConstants(layout, stageFlags, offset,size, pValues);
	}
}