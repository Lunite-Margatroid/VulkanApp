#pragma once
#include "LunarTextIO.hpp"

#include <vector>
#include <span>
#include <map>

#include "ImgRes.hpp"

namespace LTIO {
#define TEXTTABLE_TCHAR_char	0
#define TEXTTABLE_TCHAR_wchar_t	1

#define TEXTTABLE_ENCODING_UTF8		0
#define TEXTTABLE_ENCODING_UTF16	1

template<typename TChar>
class TextTable {
	protected:
		std::vector<GlyphsMetrics> m_vecGlyphsMetrics;
		std::map<TChar, uint32_t> m_mapGlyphs;

		LIIO::ImgRes m_imgTable;

		uint32_t m_nRowCount;
		uint32_t m_nColCount;

		uint32_t m_nCellWidth;
		uint32_t m_nCellHeight;

	public:
		TextTable(std::span<TChar> data, uint32_t nTextResolution);
		TextTable(const TextTable& other) = delete;
		TextTable& operator=(const TextTable& other) = delete;
		TextTable(TextTable&& other) noexcept = default;
		TextTable& operator=(TextTable&& other) noexcept = default;
		virtual ~TextTable();

		const LIIO::ImgRes& GetImage() const {
			return m_imgTable;
		}
	};

	template class TextTable<char>;
	template class TextTable<wchar_t>;

	template<>
	TextTable<char>::TextTable(std::span<char> data, uint32_t nTextResolution);

	template<>
	TextTable<char>::~TextTable();

	template<>
	TextTable<wchar_t>::TextTable(std::span<wchar_t> data, uint32_t nTextResolution);

	template<>
	TextTable<wchar_t>::~TextTable();
}