#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"
#include "PipelineLog.hpp"

#include "ShaderResourceManager.hpp"

#include "EntityRenderLogLine.hpp"

namespace LT {

	PipelineLog::PipelineLog()
		: m_nTextTable(INVALID_IMAGE_ID)
		, m_bLogLineDirt(true)
	{
		for (int i = 0; i < m_arrTransBuffer.size(); ++i)
		{
			m_arrTransBuffer[i] = ShaderResourceManager::CreateCommonConstBuffer(sizeof(VPMatrixBuffer2D), nullptr);
		}

		vk::Device& device = vkContext::GetVkDevice();


		// Init TextTable
		std::vector<char> ascii;
		for (char c = 32; c < 127; ++c)
		{
			ascii.push_back(c);
		}

		LTIO::TextTable<char> textTable(ascii, LOG_PASS_DEFAULT_RESOLUTION);
		const LIIO::ImgRes& img = textTable.GetImage();
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


		SetPrimitiveTopology(m_nLogLineFlag, vk::PrimitiveTopology::eTriangleList);
		SetPolygonMode(m_nLogLineFlag, vk::PolygonMode::eFill);
		SetBackCull(m_nLogLineFlag, false);
		SetBlendEnable(m_nLogLineFlag, true);
		// UV0 2维
		m_nLogLineFlag = m_nLogLineFlag | (static_cast<uint64_t>(VertexChannel::UV) << 0);
		// UV1 2维
		m_nLogLineFlag = m_nLogLineFlag | (static_cast<uint64_t>(VertexChannel::UV) << 1);

		PushLog("Hello World!");
	}

	PipelineLog::~PipelineLog()
	{}

	void PipelineLog::PushLog(std::string_view strLine)
	{
		m_bLogLineDirt = true;

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

		VertexBuffer* pVertexBuffer = BufferManager::CreateVertexBuffer(vecBuffer.size() * sizeof(TextQuadVertex), vecBuffer.data(), nCharCount * 6);
		BufferID nVertexBuffer = pVertexBuffer->GetBufferID();
		EntityRef refEntity = EntityManager::CreateEntityRenderLogLine();
		auto* pEntity = reinterpret_cast<EntityRenderLogLine*>(refEntity.GetPtr());
		pEntity->SetVertexBuffer(pVertexBuffer);
		pEntity->SetRenderPassFlag(m_nLogLineFlag);
		m_listLogLine.emplace_back(nVertexBuffer, refEntity);


		if (m_listLogLine.size() > LOG_PASS_MAX_LINE)
		{
			BufferManager::DeleteBuffer(m_listLogLine.front().first);
			m_listLogLine.pop_front();
		}
	}

	void PipelineLog::Resize(uint32_t nWidth, uint32_t nHeight) {
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}

	void PipelineLog::Execute(PipelineExecuteInfo& sInOutPipelineInfo) {
		FlightFrameIndex nFlightFrameIndex = sInOutPipelineInfo.nFightFrameIndex;
		ConstBufferHandle nTransBuffer = m_arrTransBuffer[nFlightFrameIndex];

		ShaderResourceManager::UpdateCommonConstBuffer(nTransBuffer,&m_sTransBuffer);

		if (m_bLogLineDirt)
		{
			m_vecLogLine.clear();
			for (auto& [nBufferID, refEntity] : m_listLogLine)
			{
				m_vecLogLine.push_back(reinterpret_cast<EntityRender*>(refEntity.GetPtr()));
			}
			m_bLogLineDirt = false;
		}

		StageExecuteInfo sStageExecuteInfo;
		sStageExecuteInfo.nWidth = m_nWidth;
		sStageExecuteInfo.nHeight = m_nHeight;
		sStageExecuteInfo.nFlightFrameIndex = nFlightFrameIndex;
		sStageExecuteInfo.nDepthBufferID = INVALID_IMAGE_ID;
		sStageExecuteInfo.vecRenderTarget = sInOutPipelineInfo.vecRenderTargets;
		sStageExecuteInfo.mapRenderEntity[MATERIAL_ID_2D_0] = m_vecLogLine;
		sStageExecuteInfo.pRenderView = nullptr;
		sStageExecuteInfo.vecSemWait = sInOutPipelineInfo.vecSemWait;
		sStageExecuteInfo.vecSemWaitMask = sInOutPipelineInfo.vecSemWaitMask;
		sStageExecuteInfo.vecSemSignal = sInOutPipelineInfo.vecSemSignal;

		sStageExecuteInfo.pPipeline = this;
		
		sStageExecuteInfo.vecColorClear  = sInOutPipelineInfo.vecColorClear;

		sStageExecuteInfo.bDepthClear = false;

		m_stageLog.Execute(sStageExecuteInfo);

		if (sStageExecuteInfo.refFence)
		{
			sInOutPipelineInfo.refFence = sStageExecuteInfo.refFence;
		}
	}

	void PipelineLog::SetColorInType(const std::vector<PipelineIOType>& vecColorInType)
	{
		Pipeline::SetColorInType(vecColorInType);
		m_stageLog.SetColorInType(vecColorInType);
	}

	void PipelineLog::SetColorOutType(const std::vector<PipelineIOType>&vecColorOutType)
	{
		Pipeline::SetColorOutType(vecColorOutType);
		m_stageLog.SetColorOutType(vecColorOutType);
	}

	void PipelineLog::SetDepthInType(PipelineIOType eDepthInType)
	{
		Pipeline::SetDepthInType(eDepthInType);
		m_stageLog.SetDepthInType(eDepthInType);
	}

	void PipelineLog::SetDepthOutType(PipelineIOType eDepthOutType)
	{
		Pipeline::SetDepthOutType(eDepthOutType);
		m_stageLog.SetDepthOutType(eDepthOutType);
	}

} // namespace LT
