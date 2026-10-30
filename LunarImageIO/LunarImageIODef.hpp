#pragma once

namespace LIIO {
	// ImageFormat 的位域布局 (低位在前):
	//   bit 0-2: 通道数      (1/2/3/4)
	//   bit 3-5: 每通道字节数 (1/2/3/4)
	//   bit 6  : 数值类型     (0 = 无符号整型, 1 = 浮点)
	//   bit 7+ : 保留

	// 通道数
	constexpr int ImageFormat_CHANNEL_SHIFT = 0;
	constexpr int ImageFormat_CHANNEL_MASK = 0x7 << ImageFormat_CHANNEL_SHIFT;
	constexpr int ImageFormat_CHANNEL1 = 1 << ImageFormat_CHANNEL_SHIFT;
	constexpr int ImageFormat_CHANNEL2 = 2 << ImageFormat_CHANNEL_SHIFT;
	constexpr int ImageFormat_CHANNEL3 = 3 << ImageFormat_CHANNEL_SHIFT;
	constexpr int ImageFormat_CHANNEL4 = 4 << ImageFormat_CHANNEL_SHIFT;

	// 每通道字节数
	constexpr int ImageFormat_BYTES_SHIFT = 3;
	constexpr int ImageFormat_BYTES_MASK = 0x7 << ImageFormat_BYTES_SHIFT;
	constexpr int ImageFormat_1BYTE_PER_CHANNEL = 1 << ImageFormat_BYTES_SHIFT;
	constexpr int ImageFormat_2BYTE_PER_CHANNEL = 2 << ImageFormat_BYTES_SHIFT;
	constexpr int ImageFormat_3BYTE_PER_CHANNEL = 3 << ImageFormat_BYTES_SHIFT;
	constexpr int ImageFormat_4BYTE_PER_CHANNEL = 4 << ImageFormat_BYTES_SHIFT;

	// 数值类型
	constexpr int ImageFormat_TYPE_SHIFT = 6;
	constexpr int ImageFormat_TYPE_MASK = 0x1 << ImageFormat_TYPE_SHIFT;
	constexpr int ImageFormat_UNSIGNED = 0 << ImageFormat_TYPE_SHIFT;
	constexpr int ImageFormat_FLOAT = 1 << ImageFormat_TYPE_SHIFT;

	enum class ImageFormat : int {
		R8 = ImageFormat_CHANNEL1 | ImageFormat_1BYTE_PER_CHANNEL | ImageFormat_UNSIGNED,
		RGB8 = ImageFormat_CHANNEL3 | ImageFormat_1BYTE_PER_CHANNEL | ImageFormat_UNSIGNED,
		RGBA8 = ImageFormat_CHANNEL4 | ImageFormat_1BYTE_PER_CHANNEL | ImageFormat_UNSIGNED,

		R16_U = ImageFormat_CHANNEL1 | ImageFormat_2BYTE_PER_CHANNEL | ImageFormat_UNSIGNED,
		R16_F = ImageFormat_CHANNEL1 | ImageFormat_2BYTE_PER_CHANNEL | ImageFormat_FLOAT,
		RGB16_U = ImageFormat_CHANNEL3 | ImageFormat_2BYTE_PER_CHANNEL | ImageFormat_UNSIGNED,
		RGBA16_U = ImageFormat_CHANNEL4 | ImageFormat_2BYTE_PER_CHANNEL | ImageFormat_UNSIGNED,
		RGB16_F = ImageFormat_CHANNEL3 | ImageFormat_2BYTE_PER_CHANNEL | ImageFormat_FLOAT,
		RGBA16_F = ImageFormat_CHANNEL4 | ImageFormat_2BYTE_PER_CHANNEL | ImageFormat_FLOAT,

		R32_F = ImageFormat_CHANNEL1 | ImageFormat_4BYTE_PER_CHANNEL | ImageFormat_FLOAT,
		RGB32_F = ImageFormat_CHANNEL3 | ImageFormat_4BYTE_PER_CHANNEL | ImageFormat_FLOAT,
		RGBA32_F = ImageFormat_CHANNEL4 | ImageFormat_4BYTE_PER_CHANNEL | ImageFormat_FLOAT,
	};

	// ---- 位域提取 ----

	// 通道数 (1/2/3/4)
	constexpr int GetChannelCount(ImageFormat eFormat) {
		return (static_cast<int>(eFormat) & ImageFormat_CHANNEL_MASK) >> ImageFormat_CHANNEL_SHIFT;
	}

	// 每通道字节数 (1/2/3/4)
	constexpr int GetBytesPerChannel(ImageFormat eFormat) {
		return (static_cast<int>(eFormat) & ImageFormat_BYTES_MASK) >> ImageFormat_BYTES_SHIFT;
	}

	// 每通道位数 (8/16/24/32)
	constexpr int GetBitsPerChannel(ImageFormat eFormat) {
		return GetBytesPerChannel(eFormat) * 8;
	}

	// 每像素字节数 (通道数 * 每通道字节数)
	constexpr int GetBytesPerPixel(ImageFormat eFormat) {
		return GetChannelCount(eFormat) * GetBytesPerChannel(eFormat);
	}

	constexpr bool IsFloatFormat(ImageFormat eFormat) {
		return (static_cast<int>(eFormat) & ImageFormat_FLOAT) != 0;
	}

	constexpr bool IsUnsignedFormat(ImageFormat eFormat) {
		return !IsFloatFormat(eFormat);
	}

	// 通道数与每通道字节数均非零
	constexpr bool IsValidImageFormat(ImageFormat eFormat) {
		return GetChannelCount(eFormat) > 0 && GetBytesPerChannel(eFormat) > 0;
	}

	struct Color {
		float r, g, b;
		Color()
			: r(0.f)
			, g(0.f)
			, b(0.f)
		{}

		Color(float rr, float gg, float bb)
			: r(rr)
			, g(gg)
			, b(bb)
		{
		}

		Color(float gray)
			: r(gray)
			, g(gray)
			, b(gray)
		{
		}
	};

	struct AColor {
		Color color;
		float alpha;

		AColor() : alpha(1.f){}
		AColor(float r, float g, float b, float a = 1.f) : color(r, g, b), alpha(a){}
		AColor(float gray) : color(gray), alpha(1.f) {}
	};
}// namespace LIIO
