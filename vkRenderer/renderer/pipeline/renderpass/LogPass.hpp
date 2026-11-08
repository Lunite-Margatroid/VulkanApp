#pragma once
#include "GraphicPass.hpp"
#include "DynamicVertexBuffer.hpp"
#include "LunarTextIO.hpp"

namespace LT {
	constexpr uint32_t LOG_PASS_MAX_LINE = 64u;
	constexpr uint32_t LOG_PASS_MAX_CHAR_COUNT = 128u;

	class LogPass : public GraphicPass {
	protected:
		std::list<BufferID> m_listVertexBuffer;
		std::list<std::string> m_listLog;

		ImageID m_nTextTable;
		
		std::vector<LTIO::GlyphsMetrics> m_vecMetrics;

	public:
		LogPass();
		~LogPass();
		
		void Push(std::string_view strLine);
	};

}// namespace LT