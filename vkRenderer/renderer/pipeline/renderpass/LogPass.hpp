#pragma once
#include "GraphicPass.hpp"
#include "DynamicVertexBuffer.hpp"
#include "LunarTextIO.hpp"

namespace LT {
	constexpr uint32_t LOG_PASS_MAX_LINE = 64u;
	constexpr uint32_t LOG_PASS_MAX_CHAR_COUNT = 128u;
	constexpr uint32_t LOG_PASS_DEFAULT_RESOLUTION = 64u;
	constexpr uint32_t LOG_PASS_DEFAULT_LINE_HEIGHT = 64u;

	// 绘制log要使用的push constant
	struct LogPassInfo {
		
	};


	class LogPass : public GraphicPass {
		struct TextRect {
			float x, y;
			float width, height;
		};

	protected:
		// 每行文字使用的顶点缓冲
		std::list<BufferID> m_listVertexBuffer;
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

	public:
		LogPass();
		~LogPass();
		
		void Push(std::string_view strLine);
	};

}// namespace LT