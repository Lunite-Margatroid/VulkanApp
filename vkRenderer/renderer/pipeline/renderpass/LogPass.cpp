#include "vkRendererCommon.h"
#include "vkContext.h"
#include "LogPass.hpp"

#include "BufferManager.h"

#include "ImageManager.h"
#include "TextTable.hpp"

namespace LT {
	LogPass::LogPass()
		: m_nTextTable(INVALID_IMAGE_ID)
	{
		vk::Device& device = vkContext::GetVkDevice();

		m_listVertexBuffer.push_back(BufferManager::CreateDynamicVertexBuffer(8096, nullptr, 0)->GetBufferID());
		
		SetPrimitiveTopology(m_nFlag, vk::PrimitiveTopology::eTriangleList);
		SetPolygonMode(m_nFlag, vk::PolygonMode::eFill);
		SetBackCull(m_nFlag, false);
		SetBlendEnable(m_nFlag, true);
		// UV0 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 0);
		// UV1 2维
		m_nFlag = m_nFlag | (static_cast<uint64_t>(VertexChannel::UV) << 1);


		// Init TextTable
		std::vector<char> ascii;
		for (char c = 32; c < 127; ++c)
		{
			ascii.push_back(c);
		}

		LTIO::TextTable<char> textTable(ascii, LOG_PASS_DEFAULT_RESOLUTION);
		const LIIO::ImgRes &img = textTable.GetImage();
		Image2DShaderRes* pImg = ImageManager::CreateImage2DShaderResource(vk::Format::eR8Unorm, img.GetWidth(), img.GetHeight());
		pImg->AssignMemory(img.GetDataPtr(), img.GetPixelCount() * LIIO::GetBytesPerPixel(img.GetFormat()));
		m_nTextTable = pImg->GetImageID();
		m_vecMetrics = textTable.GetGlyphsMetrics();

		float fCellWidth = 1.f / textTable.GetColCount();
		float fCellHeight = 1.f / textTable.GetRowCount();
		for (int i = 0; i < ascii.size(); ++i)
		{
			m_mapGlyphs[ascii[i]] = i;

			int col = i % textTable.GetColCount();
			int row = i / textTable.GetColCount();

			TextRect rect;
			rect.x = col * fCellWidth;
			rect.y = row * fCellHeight;
			rect.width = m_vecMetrics[i].width / img.GetWidth();
			rect.height = m_vecMetrics[i].height / img.GetHeight();
			m_vecTextRects.push_back(rect);
		}
		// 每行文本的高度(像素)
		m_fCellHeight = textTable.GetCellHeight();

		// Init Pipeline
		std::vector<vk::DescriptorSetLayout> layouts;

		vk::DescriptorSetLayoutBinding binding = {};
		binding
			.setBinding(0)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			;

		vk::DescriptorSetLayoutCreateInfo dslci = {};
		dslci
			.setPBindings(&binding)
			.setBindingCount(1)
			;
		layouts.push_back(device.createDescriptorSetLayout(dslci));

		std::vector<vk::PushConstantRange> pushConstantRanges;
		vk::PushConstantRange pcr;
		pcr
			.setOffset(0)
			.setSize(sizeof(float) * 2)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);
		pushConstantRanges.push_back(pcr);
		Init(layouts, pushConstantRanges);
	}

	LogPass::~LogPass() {
		for (BufferID vertexBuffer: m_listVertexBuffer)
		{
			BufferManager::DeleteBuffer(vertexBuffer);
		}
	}
	void LogPass::Push(std::string_view strLine)
	{
		size_t nCharCount = std::min(strLine.size(), static_cast<size_t>(LOG_PASS_MAX_CHAR_COUNT));
		
		struct TextQuadVertex {
			float posX;
			float posY;
			float uvX;
			float uvY;
		};

		std::vector<TextQuadVertex> vecBuffer(nCharCount * 6);
		float cursorX = 0.f;
		float cursorY = m_fCellHeight / 3.f;

		const uint32_t nStridePerChar = 6;
		for (size_t i = 0; i < nCharCount; ++i)
		{
			// Process each character
			char c = strLine[i];
			uint32_t index = m_mapGlyphs[c];
			TextRect& rect = m_vecTextRects[index];
			LTIO::GlyphsMetrics& metrics = m_vecMetrics[index];

			float uvLeft = rect.x;
			float uvBottom = rect.y + rect.height;
			float uvRight = rect.x + rect.width;
			float uvTop = rect.y;

			float posLeft = cursorX + metrics.horiBearingX;
			float posRight = cursorX + metrics.horiBearingX + metrics.width;
			float posBottom = cursorY - (metrics.height - metrics.horiBearingY);
			float posTop = cursorY + metrics.horiBearingY;

			// 6 vertices for the quad
			// leftBottom
			{
				TextQuadVertex& vertex = vecBuffer[i * nStridePerChar];
				vertex.posX = posLeft;
				vertex.posY = posBottom;
				vertex.uvX = uvLeft;
				vertex.uvY = uvBottom;
			}
			// RightTop
			{
				TextQuadVertex& vertex = vecBuffer[i * nStridePerChar + 1];
				vertex.posX = posRight;
				vertex.posY = posTop;
				vertex.uvX = uvRight;
				vertex.uvY = uvTop;
			}
			// LeftTop
			{
				TextQuadVertex& vertex = vecBuffer[i * nStridePerChar + 2];
				vertex.posX = posLeft;
				vertex.posY = posTop;
				vertex.uvX = uvLeft;
				vertex.uvY = uvTop;
			}
			// leftBottom
			{
				TextQuadVertex& vertex = vecBuffer[i * nStridePerChar + 3];
				vertex.posX = posLeft;
				vertex.posY = posBottom;
				vertex.uvX = uvLeft;
				vertex.uvY = uvBottom;
			}
			// RightBottom
			{
				TextQuadVertex& vertex = vecBuffer[i * nStridePerChar + 4];
				vertex.posX = posRight;
				vertex.posY = posBottom;
				vertex.uvX = uvRight;
				vertex.uvY = uvBottom;
			}
			// RightTop
			{
				TextQuadVertex& vertex = vecBuffer[i * nStridePerChar + 5];
				vertex.posX = posRight;
				vertex.posY = posTop;
				vertex.uvX = uvRight;
				vertex.uvY = uvTop;
			}

			cursorX += metrics.horiAdvance;
		}

		m_listVertexBuffer.push_back(BufferManager::CreateVertexBuffer(vecBuffer.size() * sizeof(TextQuadVertex), vecBuffer.data(), nCharCount * 6)->GetBufferID());
		if(m_listVertexBuffer.size() > LOG_PASS_MAX_LINE)
		{
			BufferManager::DeleteBuffer(m_listVertexBuffer.front());
			m_listVertexBuffer.pop_front();
		}
	}
}