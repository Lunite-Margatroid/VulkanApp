#pragma once

namespace LTIO {
	void LunarTestIO_Test();

	struct GlyphsMetrics {
		float width;
		float height;

		float horiBearingX;
		float horiBearingY;
		float horiAdvance;

		float vertBearingX;
		float vertBearingY;
		float vertAdvance;

		GlyphsMetrics() 
			: width(0.f)
			, height(0.f)
			, horiBearingX(0.f)
			, horiBearingY(0.f)
			, horiAdvance(0.f)
			, vertBearingX(0.f)
			, vertBearingY(0.f)
			, vertAdvance(0.f)
		{}
	};
}