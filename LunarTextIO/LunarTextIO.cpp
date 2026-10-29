#include "LunarTextIO.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "harfbuzz/hb.h"
#include "harfbuzz/hb-ft.h"
#include <ctime>

#include "logger.hpp"

namespace LT
{
	void LunarTestIO_Test() {
		FT_Library ftLibrary;
		FT_Error err = FT_Init_FreeType(&ftLibrary);
		
		RENDERER_ASSERT(err == 0, "FreeType Init Failed.");

		FT_Face ftFace;
		// 注意：HarfBuzz 需要字体有具体大小，FT_Set_Char_Size 必须调用
		err = FT_New_Face(ftLibrary, "C:/Windows/Fonts/simhei.ttf", 0, &ftFace);
		RENDERER_ASSERT(err == 0, "Face Created Failed.");

		FT_Set_Char_Size(ftFace, 0, 16 * 64, 0, 0); // 16pt，26.6 定点数

		hb_font_t* hbFont = hb_ft_font_create_referenced(ftFace);
		// 若使用非 referenced 版本，需自行管理 FT_Face 生命周期

		hb_buffer_t* buf = hb_buffer_create();
		const char* text = "Hello";
		hb_buffer_add_utf8(buf, text, -1, 0, -1);

		// 自动推测文本方向、脚本和语言（适用于大多数场景）
		hb_buffer_guess_segment_properties(buf);

		// 核心调用：整形
		hb_shape(hbFont, buf, NULL, 0);

		unsigned int glyphCount;
		hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);
		hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buf, &glyphCount);

		// 遍历每个字形，准备交给 Vulkan 绘制
		for (unsigned int i = 0; i < glyphCount; i++) {
			hb_codepoint_t glyphIndex = glyphInfo[i].codepoint; // 字形 ID
			// 注意：位置是 26.6 定点数，需除以 64.0 转为像素/浮点
			float xOffset = glyphPos[i].x_offset / 64.0f;
			float yOffset = glyphPos[i].y_offset / 64.0f;
			float xAdvance = glyphPos[i].x_advance / 64.0f;
			float yAdvance = glyphPos[i].y_advance / 64.0f;
			// 这里可以将 glyphIndex 和计算后的位置传给 SDF 生成或四边形顶点构建逻辑
		}

		hb_buffer_destroy(buf);
		hb_font_destroy(hbFont); // 会适当减少对 FT_Face 的引用
		// 若使用了 hb_ft_font_create_referenced，此处还需 FT_Done_Face(ftFace)
	}
}