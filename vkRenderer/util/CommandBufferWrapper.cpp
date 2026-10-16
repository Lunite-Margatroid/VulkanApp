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
		}

		for (CmdBufferWithFence& cmdBufferWithFence : m_arrCmdBuffers)
		{
			m_qCmdBuffers.push(&cmdBufferWithFence);
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
		CmdBufferWithFence* cmdBufferWithFence = m_qCmdBuffers.front();

		vk::SubmitInfo si = {};
		si
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmdBufferWithFence->cmdBuffer)
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
			cmdBufferWithFence->refFence.GetNativeFence()
		);

		m_qCmdBuffers.pop();
		m_qCmdBuffers.push(cmdBufferWithFence);

		return cmdBufferWithFence->refFence;
	}
	void CommandBufferWrapper::PipelineBarrier2(const vk::DependencyInfo sDependencyInfo)
	{
		m_qCmdBuffers.front()->cmdBuffer.pipelineBarrier2(sDependencyInfo);
	}
	void CommandBufferWrapper::Begin(const vk::CommandBufferBeginInfo& sBeginInfo)
	{
		// 等待上一次提交的Fence
		if (!m_qCmdBuffers.front()->refFence)
		{
			m_qCmdBuffers.front()->refFence = FenceManager::CreateFence();
		}
		else
		{
			m_qCmdBuffers.front()->refFence.WaitAndResetFence();
		}
		
		m_qCmdBuffers.front()->cmdBuffer.begin(sBeginInfo);
	}
	void CommandBufferWrapper::BeginRendering(const vk::RenderingInfo& sRenderingInfo)
	{
		m_qCmdBuffers.front()->cmdBuffer.beginRendering(sRenderingInfo);
	}
	void CommandBufferWrapper::BindPipeline(const vk::PipelineBindPoint& pipelineBindPoint, const vk::Pipeline pipeline)
	{
		m_qCmdBuffers.front()->cmdBuffer.bindPipeline(pipelineBindPoint, pipeline);
	}
	void CommandBufferWrapper::BindVertexBuffers(uint32_t firstBinding, const std::vector<vk::Buffer> buffers, const std::vector<vk::DeviceSize>& offsets)
	{
		m_qCmdBuffers.front()->cmdBuffer.bindVertexBuffers(firstBinding, buffers, offsets);
	}
	void CommandBufferWrapper::BindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType)
	{
		m_qCmdBuffers.front()->cmdBuffer.bindIndexBuffer(buffer, offset, indexType);
	}
	void CommandBufferWrapper::BindDescriptorSets(
		vk::PipelineBindPoint pipelineBindPoint,
		const vk::PipelineLayout& layout,
		uint32_t firstSet,
		const std::vector<vk::DescriptorSet>& descriptorSets,
		const std::vector<uint32_t>& dynamicOffsets)
	{
		m_qCmdBuffers.front()->cmdBuffer.bindDescriptorSets(pipelineBindPoint, layout, firstSet, descriptorSets, dynamicOffsets);
	}
	void CommandBufferWrapper::SetViewport(uint32_t firstViewport, const std::vector<vk::Viewport>& viewports)
	{
		m_qCmdBuffers.front()->cmdBuffer.setViewport(firstViewport, viewports);
	}
	void CommandBufferWrapper::SetScissor(uint32_t firstScissor, const std::vector<vk::Rect2D>& scissors)
	{
		m_qCmdBuffers.front()->cmdBuffer.setScissor(firstScissor, scissors);
	}
	void CommandBufferWrapper::DrawIndexed(
		uint32_t         indexCount,
		uint32_t         instanceCount,
		uint32_t         firstIndex,
		int32_t          vertexOffset,
		uint32_t         firstInstance)
	{
		m_qCmdBuffers.front()->cmdBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	void CommandBufferWrapper::EndRendering()
	{
		m_qCmdBuffers.front()->cmdBuffer.endRendering();
	}
	void CommandBufferWrapper::End()
	{
		m_qCmdBuffers.front()->cmdBuffer.end();
	}
}