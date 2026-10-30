#include "LunarTextIO.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "harfbuzz/hb.h"
#include "harfbuzz/hb-ft.h"
#include <ctime>

#include "logger.hpp"

#include "LunarImageIO.hpp"

#include <string>
#include <vector>
#include <algorithm>

#include <random>
namespace LT
{
	void LunarTestIO_Test() {
		FT_Library ftLibrary;
		FT_Error err = FT_Init_FreeType(&ftLibrary);
		
		RENDERER_ASSERT(err == 0, "FreeType Init Failed.");

		FT_Face ftFace;
		// 注意：HarfBuzz 需要字体有具体大小，FT_Set_Char_Size 必须调用
		err = FT_New_Face(ftLibrary, "C:\\Windows\\Fonts\\CascadiaCode.ttf", 0, &ftFace);
		RENDERER_ASSERT(err == 0, "Face Created Failed.");

		FT_Set_Char_Size(ftFace, 0, 16 * 64, 300, 300); // 16pt，26.6 定点数

		FT_Set_Pixel_Sizes(ftFace, 0, 64);

		hb_font_t* hbFont = hb_ft_font_create_referenced(ftFace);
		// 若使用非 referenced 版本，需自行管理 FT_Face 生命周期

		hb_buffer_t* buf = hb_buffer_create();
		const char* text = "The quick brown fox jumps over the lazy dog==";
		hb_buffer_add_utf8(buf, text, -1, 0, -1);

		// 自动推测文本方向、脚本和语言（适用于大多数场景）
		hb_buffer_guess_segment_properties(buf);

		// 核心调用：整形
		hb_shape(hbFont, buf, NULL, 0);

		hb_feature_t feat;
		feat.value;

		HB_FEATURE_GLOBAL_END;

		unsigned int glyphCount;
		hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);
		hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buf, &glyphCount);


		std::vector<LIIO::ImgRes> vecChars;
		std::vector<FT_Glyph_Metrics> vecMetrics;
		float nLineWidth = 0;
		float nLineHeight = 0;
		int cursor_y = 10;

		float fMaxBearingYPlus = 0.0;
		float fMaxBearingYMinus = 0.0;

		// 遍历每个字形，准备交给 Vulkan 绘制
		for (unsigned int i = 0; i < glyphCount; i++) {
			hb_codepoint_t glyphIndex = glyphInfo[i].codepoint; // 字形 ID
			// 注意：位置是 26.6 定点数，需除以 64.0 转为像素/浮点
			float xOffset = glyphPos[i].x_offset / 64.0f;
			float yOffset = glyphPos[i].y_offset / 64.0f;
			float xAdvance = glyphPos[i].x_advance / 64.0f;
			float yAdvance = glyphPos[i].y_advance / 64.0f;
			LOG_INFO("GlyphPos[%d]: (%f, %f, %f, %f)", i, xOffset, yOffset, xAdvance, yAdvance);
			// 这里可以将 glyphIndex 和计算后的位置传给 SDF 生成或四边形顶点构建逻辑

			

			FT_Load_Glyph(
				ftFace,
				glyphIndex,
				FT_LOAD_DEFAULT
				);

			LOG_INFO("Metrics Info:\n\
width: %f\n\
height: %f\n\
horiAdvance: %f\n\
vertAdvance: %f\n\
horiBearingX: %f\n\
horiBearingY: %f\n\
vertBearingX: %f\n\
vertBearingY: %f\n"
,
ftFace->glyph->metrics.width / 64.f,
ftFace->glyph->metrics.height / 64.f,
ftFace->glyph->metrics.horiAdvance / 64.f,
ftFace->glyph->metrics.vertAdvance / 64.f,
ftFace->glyph->metrics.horiBearingX / 64.f,
ftFace->glyph->metrics.horiBearingY / 64.f,
ftFace->glyph->metrics.vertBearingX / 64.f,
ftFace->glyph->metrics.vertBearingY / 64.f
);
			fMaxBearingYPlus = std::max(ftFace->glyph->metrics.vertBearingY / 64.f, fMaxBearingYPlus);
			fMaxBearingYMinus = std::max(ftFace->glyph->metrics.height / 64.f - ftFace->glyph->metrics.vertBearingY / 64.f, fMaxBearingYMinus);

			FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

			LIIO::ImgRes img(ftFace->glyph->bitmap.buffer, ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows, LIIO::ImageFormat::R8);
			
			nLineWidth += ftFace->glyph->metrics.horiAdvance / 64.f;
			

			std::string imgPath = "./testImage" + std::to_string(i) + ".png";

			vecMetrics.push_back(ftFace->glyph->metrics);

			cursor_y = std::max(static_cast<int>(ftFace->glyph->metrics.horiBearingY / 64.f + 0.5f), cursor_y);

			img.SavaImage(imgPath.c_str());

			vecChars.push_back(std::move(img));
		}

		nLineHeight = fMaxBearingYPlus + fMaxBearingYMinus + 0.5;
		LIIO::ImgRes imgFinal(static_cast<int>(nLineWidth + 0.5f) + 20, static_cast<int>(nLineHeight + 0.5f) + 20, LIIO::ImageFormat::R8);


		int cursor_x = 0;
		for (int i = 0; i < glyphCount; ++i)
		{	
			float xOffset = glyphPos[i].x_offset / 64.0f;
			float yOffset = glyphPos[i].y_offset / 64.0f;
			float xAdvance = glyphPos[i].x_advance / 64.0f;
			float yAdvance = glyphPos[i].y_advance / 64.0f;

			int nDstY = nLineHeight - (cursor_y + (vecMetrics[i].height - vecMetrics[i].vertBearingY) / 64);
			int nSrcX = 0;
			int nSrcY = 0;
			imgFinal.CopyRegion(vecChars[i], cursor_x + xOffset, nDstY + yOffset, nSrcX, nSrcY, vecChars[i].GetWidth(), vecMetrics[i].vertAdvance / 64);

			cursor_x += static_cast<int>(xAdvance + 0.5f);
		}

		imgFinal.SavaImage("./testString.png");

		hb_buffer_destroy(buf);
		hb_font_destroy(hbFont); // 会适当减少对 FT_Face 的引用
		// 若使用了 hb_ft_font_create_referenced，此处还需 FT_Done_Face(ftFace)
	}
}