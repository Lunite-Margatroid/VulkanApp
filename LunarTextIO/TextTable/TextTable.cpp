#include "LunarTextIOCommon.hpp"
#include "TextTable.hpp"
#include <cmath>
#include <ctime>

#include "logger.hpp"

namespace LTIO {

	template<>
	inline TextTable<char>::TextTable(std::span<char> data, uint32_t nTextResolution)
		: m_nCellHeight(0), m_nCellWidth(0), m_imgTable()
	{
		m_nColCount = static_cast<uint32_t>(std::sqrt(data.size()));
		m_nRowCount = static_cast<uint32_t>(std::ceil(static_cast<float>(data.size()) / m_nColCount));

		// 初始化FreeType
		FT_Library ftLibrary;
		FT_Error ftErr = FT_Init_FreeType(&ftLibrary);

		RENDERER_ASSERT(ftErr == 0, "FreeType Init Failed.");

		// 加载Face
		FT_Face ftFace;
		ftErr = FT_New_Face(ftLibrary, "C:\\Windows\\Fonts\\CascadiaCode.ttf", 0, &ftFace);

		RENDERER_ASSERT(ftErr == 0, "FreeType Face Load Failed.");

		ftErr = FT_Set_Char_Size(ftFace, 0, 16 * 64, 96, 96);
		RENDERER_ASSERT(ftErr == 0, "FreeType Char Size Set Failed.");

		ftErr = FT_Set_Pixel_Sizes(ftFace, 0, nTextResolution);
		RENDERER_ASSERT(ftErr == 0, "FreeType Pixel Size Set Failed.");

		std::vector<LIIO::ImgRes> vecImgChars;
		for (char charTemp : data) {

			uint32_t nCharIndex = FT_Get_Char_Index(ftFace, charTemp);

			if (nCharIndex != 0) {
				ftErr = FT_Load_Glyph(ftFace, nCharIndex, FT_LOAD_DEFAULT);
				RENDERER_ASSERT(ftErr == 0, "FreeType Glyph Load Failed.");
				FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

				GlyphsMetrics metrics;
				metrics.width = static_cast<float>(ftFace->glyph->metrics.width) / 64.f;
				metrics.height = static_cast<float>(ftFace->glyph->metrics.height) / 64.f;
				metrics.horiAdvance = static_cast<float>(ftFace->glyph->metrics.horiAdvance) / 64.f;
				metrics.horiBearingX = static_cast<float>(ftFace->glyph->metrics.horiBearingX) / 64.f;
				metrics.horiBearingY = static_cast<float>(ftFace->glyph->metrics.horiBearingY) / 64.f;
				metrics.vertAdvance = static_cast<float>(ftFace->glyph->metrics.vertAdvance) / 64.f;
				metrics.vertBearingX = static_cast<float>(ftFace->glyph->metrics.vertBearingX) / 64.f;
				metrics.vertBearingY = static_cast<float>(ftFace->glyph->metrics.vertBearingY) / 64.f;

				m_vecGlyphsMetrics.push_back(metrics);

				vecImgChars.emplace_back(ftFace->glyph->bitmap.buffer, ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows, LIIO::ImageFormat::R8);

				m_nCellHeight = std::max(m_nCellHeight, ftFace->glyph->bitmap.rows);
				m_nCellWidth = std::max(m_nCellWidth, ftFace->glyph->bitmap.width);

				m_mapGlyphs[charTemp] = m_vecGlyphsMetrics.size() - 1;

			}
		}

		m_imgTable = LIIO::ImgRes(m_nCellWidth * m_nColCount, m_nCellHeight * m_nRowCount, LIIO::ImageFormat::R8);

		for (size_t i = 0; i < vecImgChars.size(); ++i) {
			// Copy character images to the main table
			if (vecImgChars[i].GetWidth() > 0 && vecImgChars[i].GetHeight() > 0) {
				m_imgTable.CopyRegion(
					vecImgChars[i],
					(i % m_nColCount) * m_nCellWidth, (i / m_nColCount) * m_nCellHeight,
					0, 0,
					vecImgChars[i].GetWidth(), vecImgChars[i].GetHeight());
			}
		}

		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLibrary);
	}

	template<>
	inline TextTable<char>::~TextTable() {
	}

	template<>
	inline TextTable<wchar_t>::TextTable(std::span<wchar_t> data, uint32_t nTextResolution)
		: m_nCellHeight(0), m_nCellWidth(0), m_imgTable()
	{
		m_nColCount = static_cast<uint32_t>(std::sqrt(data.size()));
		m_nRowCount = static_cast<uint32_t>(std::ceil(static_cast<float>(data.size()) / m_nColCount));

		// 初始化FreeType
		FT_Library ftLibrary;
		FT_Error ftErr = FT_Init_FreeType(&ftLibrary);

		RENDERER_ASSERT(ftErr == 0, "FreeType Init Failed.");

		// 加载Face
		FT_Face ftFace;
		ftErr = FT_New_Face(ftLibrary, "C:\\Windows\\Fonts\\simsun.ttc", 0, &ftFace);

		RENDERER_ASSERT(ftErr == 0, "FreeType Face Load Failed.");

		ftErr = FT_Set_Char_Size(ftFace, 0, 16 * 64, 96, 96);
		RENDERER_ASSERT(ftErr == 0, "FreeType Char Size Set Failed.");

		ftErr = FT_Set_Pixel_Sizes(ftFace, 0, nTextResolution);
		RENDERER_ASSERT(ftErr == 0, "FreeType Pixel Size Set Failed.");

		ftErr = FT_Select_Charmap(ftFace, FT_ENCODING_UNICODE);
		RENDERER_ASSERT(ftErr == 0, "FreeType Charmap Select Failed.");

		std::vector<LIIO::ImgRes> vecImgChars;
		for (wchar_t charTemp : data) {

			uint32_t nCharIndex = FT_Get_Char_Index(ftFace, charTemp);

			if (nCharIndex != 0) {
				ftErr = FT_Load_Glyph(ftFace, nCharIndex, FT_LOAD_DEFAULT);
				RENDERER_ASSERT(ftErr == 0, "FreeType Glyph Load Failed.");
				FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

				GlyphsMetrics metrics;
				metrics.width = static_cast<float>(ftFace->glyph->metrics.width) / 64.f;
				metrics.height = static_cast<float>(ftFace->glyph->metrics.height) / 64.f;
				metrics.horiAdvance = static_cast<float>(ftFace->glyph->metrics.horiAdvance) / 64.f;
				metrics.horiBearingX = static_cast<float>(ftFace->glyph->metrics.horiBearingX) / 64.f;
				metrics.horiBearingY = static_cast<float>(ftFace->glyph->metrics.horiBearingY) / 64.f;
				metrics.vertAdvance = static_cast<float>(ftFace->glyph->metrics.vertAdvance) / 64.f;
				metrics.vertBearingX = static_cast<float>(ftFace->glyph->metrics.vertBearingX) / 64.f;
				metrics.vertBearingY = static_cast<float>(ftFace->glyph->metrics.vertBearingY) / 64.f;

				m_vecGlyphsMetrics.push_back(metrics);

				vecImgChars.emplace_back(ftFace->glyph->bitmap.buffer, ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows, LIIO::ImageFormat::R8);

				m_nCellHeight = std::max(m_nCellHeight, ftFace->glyph->bitmap.rows);
				m_nCellWidth = std::max(m_nCellWidth, ftFace->glyph->bitmap.width);

				m_mapGlyphs[charTemp] = m_vecGlyphsMetrics.size() - 1;

			}
		}

		m_imgTable = LIIO::ImgRes(m_nCellWidth * m_nColCount, m_nCellHeight * m_nRowCount, LIIO::ImageFormat::R8);

		for (size_t i = 0; i < vecImgChars.size(); ++i) {
			// Copy character images to the main table
			if (vecImgChars[i].GetWidth() > 0 && vecImgChars[i].GetHeight() > 0) {
				m_imgTable.CopyRegion(
					vecImgChars[i],
					(i % m_nColCount) * m_nCellWidth, (i / m_nColCount) * m_nCellHeight,
					0, 0,
					vecImgChars[i].GetWidth(), vecImgChars[i].GetHeight());
			}
		}

		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLibrary);
	}

	template<>
	inline TextTable<wchar_t>::~TextTable() {}

}