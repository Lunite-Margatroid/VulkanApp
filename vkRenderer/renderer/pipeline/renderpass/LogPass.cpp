#include "vkRendererCommon.h"
#include "vkContext.h"
#include "LogPass.hpp"

#include "BufferManager.h"

#include "ImageManager.h"
#include "TextTable.hpp"

namespace LT {
	LogPass::LogPass()
	{
		m_listVertexBuffer.push_back(BufferManager::CreateDynamicVertexBuffer(8096, nullptr, 0)->GetBufferID());
		
		SetPrimitiveTopology(m_nFlag, vk::PrimitiveTopology::eTriangleList);
		SetPolygonMode(m_nFlag, vk::PolygonMode::eFill);
		SetBackCull(m_nFlag, false);
		SetBlendEnable(m_nFlag, true);
		// UV0 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 0);
		// UV1 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 1);


		std::vector<char> ascii;
		for (char c = 32; c < 127; ++c)
		{
			ascii.push_back(c);
		}

		LTIO::TextTable<char> textTable(ascii, 64);

		const LIIO::ImgRes &img = textTable.GetImage();

		Image2DShaderRes* pImg = ImageManager::CreateImage2DShaderResource(vk::Format::eR8Unorm, img.GetWidth(), img.GetHeight());
		
		pImg->AssignMemory(img.GetDataPtr(), img.GetPixelCount() * LIIO::GetBytesPerPixel(img.GetFormat()));
	}

	LogPass::~LogPass() {
		for (BufferID vertexBuffer: m_listVertexBuffer)
		{
			BufferManager::DeleteBuffer(vertexBuffer);
		}
	}
}