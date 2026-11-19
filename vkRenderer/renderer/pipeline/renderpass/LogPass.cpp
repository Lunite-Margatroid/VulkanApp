#include "vkRendererCommon.h"
#include "vkContext.h"
#include "LogPass.hpp"

#include "BufferManager.h"

#include "ImageManager.h"
#include "TextTable.hpp"

namespace LT {
	LogPass::LogPass()
	{
		vk::Device& device = vkContext::GetVkDevice();

		SetPrimitiveTopology(m_nFlag, vk::PrimitiveTopology::eTriangleList);
		SetPolygonMode(m_nFlag, vk::PolygonMode::eFill);
		SetBackCull(m_nFlag, false);
		SetBlendEnable(m_nFlag, true);
		SetLineWidth(m_nFlag, 1.0f);
		// UV0 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 0);
		// UV1 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 1);
	}

	LogPass::~LogPass() {
	}
}