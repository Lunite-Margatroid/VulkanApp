#pragma once
#include "GraphicPass.hpp"
#include "DynamicVertexBuffer.hpp"
#include "LunarTextIO.hpp"

namespace LT {
	constexpr uint32_t LOG_PASS_MAX_LINE = 64u;
	constexpr uint32_t LOG_PASS_MAX_CHAR_COUNT = 128u;
	constexpr uint32_t LOG_PASS_DEFAULT_RESOLUTION = 64u;
	constexpr uint32_t LOG_PASS_DEFAULT_LINE_HEIGHT = 64u;

	class LogPass : public GraphicPass {
		struct TextRect {
			float x, y;
			float width, height;
		};

	protected:
		std::list<BufferID> m_listVertexBuffer;
		std::list<std::string> m_listLog;

		ImageID m_nTextTable;
		
		std::vector<LTIO::GlyphsMetrics> m_vecMetrics;
		std::vector<TextRect> m_vecTextRects;

		std::map<char, uint32_t> m_mapGlyphs;
		
		float m_fCellHeight;

	public:
		LogPass();
		~LogPass();
		
		void Push(std::string_view strLine);
	};

}// namespace LT