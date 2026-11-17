#pragma once
#include "Pipeline.hpp"
#include "LunarTextIO.hpp"
#include "TextTable.hpp"
#include "RenderStageLog.hpp"

#include "EntityManager.hpp"

namespace LT {

	constexpr uint32_t LOG_PASS_MAX_LINE = 64u;
	constexpr uint32_t LOG_PASS_MAX_CHAR_COUNT = 128u;
	constexpr uint32_t LOG_PASS_DEFAULT_RESOLUTION = 64u;
	constexpr uint32_t LOG_PASS_DEFAULT_LINE_HEIGHT = 64u;

	class EntityRender;

	class PipelineLog : public Pipeline {
		struct TextRect {
			float x, y;
			float width, height;
		};

		// 渲染实体和顶点缓冲
		// 和m_listVertexBuffer一一对应
		std::list<std::pair<BufferID, EntityRef>> m_listLogLine;
		// 暂存log的列表
		std::list<std::string> m_listLog;
		// ascii码表
		// 用作ShaderResourceView的纹理
		ImageID m_nTextTable;
		// ascii字符的测量数据
		std::vector<LTIO::GlyphsMetrics> m_vecMetrics;
		// ascii字符的纹理坐标
		std::vector<TextRect> m_vecTextRects;
		// ascii字符的索引表
		std::map<char, uint32_t> m_mapGlyphs;
		// 在字符纹理表中 每一个字符占的格子的像素高度
		float m_fCellHeight;



	protected:
		RenderStageLog m_stageLog;
		std::array<ConstBufferHandle, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrTransBuffer;
		VPMatrixBuffer2D m_sTransBuffer;
		std::vector<EntityRender*> m_vecLogLine;
		bool m_bLogLineDirt;

		RenderPassFlag m_nLogLineFlag;


	public:
		PipelineLog();
		~PipelineLog();
		void LogPipelineInfo();

		void PushLog(std::string_view strLine);

		void Resize(uint32_t nWidth, uint32_t nHeight) override;
		void Execute(PipelineExecuteInfo& sInOutPipelineInfo) override;

		void SetColorInType(const std::vector<PipelineIOType>& vecColorInType) override;
		void SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType) override;
		void SetDepthInType(PipelineIOType eDepthInType) override;
		void SetDepthOutType(PipelineIOType eDepthOutType) override;
	};
}