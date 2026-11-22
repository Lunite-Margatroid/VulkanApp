#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "EntityRenderLogLine.hpp"

#include "VertexBuffer.h"

namespace LT {
	EntityRenderLogLine::EntityRenderLogLine(EntityID id)
		: EntityRender2D(id)
	{}
	EntityRenderLogLine::~EntityRenderLogLine()
	{}
	void EntityRenderLogLine::Draw(EntityDrawInfo& sInOutDrawInfo)
	{
		if (m_pVertexBuffer)
		{
			CommandBufferWrapper* pCmdBuffer = sInOutDrawInfo.pCommandBuffer;
			std::array<vk::Buffer, 1> arrVertexBuffer{ m_pVertexBuffer->GetNativeBuffer() };
			std::array<vk::DeviceSize, 1> arrVertexBufferOffset{ 0 };
			pCmdBuffer->BindVertexBuffers(0, arrVertexBuffer, arrVertexBufferOffset);
			pCmdBuffer->PushConstants(sInOutDrawInfo.vkLayout, vk::ShaderStageFlagBits::eVertex, 0, static_cast<uint32_t>(sizeof(ModelMat2D)), &m_matModel);
			pCmdBuffer->Draw(m_pVertexBuffer->GetVertexCount(), 1, 0, 0);
		}
	}
	void EntityRenderLogLine::SetVertexBuffer(VertexBuffer* pVertexBuffer)
	{
		m_pVertexBuffer = pVertexBuffer;
	}
} // namespace LT