#pragma once

#include "LunarImageIODef.hpp"

namespace LIIO {

	class ImgRes
	{
	private:
		ImageFormat m_Format;	// 像素格式 (通道数 / 每通道字节数 / 数值类型)
		int m_Width;
		int m_Height;
		unsigned char* m_Data;
	public:
		ImgRes(const char* path, int nTargetDepth);
		ImgRes(const unsigned char* data, int nWidth, int nHeight, ImageFormat eFormat);
		ImgRes(int nWidth, int nHeight, ImageFormat eFormat, const AColor& color = AColor());
		ImgRes();
		~ImgRes();
		ImgRes(const ImgRes&) = delete;
		ImgRes& operator = (const ImgRes&) = delete;
		ImgRes(ImgRes&& other) noexcept;
		ImgRes& operator = (ImgRes&& other) noexcept;

		inline ImageFormat GetFormat() const {
			return m_Format;
		}

		inline int GetWidth() const {
			return m_Width;
		}
		inline int GetHeight() const {
			return m_Height;
		}
		// 通道数
		inline int GetChannal() const {
			return GetChannelCount(m_Format);
		}

		// 每通道位数 (8/16/32)
		inline int GetDepth() const {
			return GetBitsPerChannel(m_Format);
		}

		inline int GetPixelCount() const
		{
			return m_Width * m_Height;
		}

		const unsigned char* GetDataPtr() const {
			return m_Data;
		}


		void Release();
		bool Resize(int newSizeWidth, int newSizeHeight);

		void SavaImage(const char* strPath, ImageFormat eFormat = ImageFormat::RGBA8) const;

		void CopyRegion(const ImgRes& other, int distX, int distY, int srcX, int srcY, int nWidth, int nHeight);

		void DrawRect(int x, int y, int width, int height, const AColor& color);
	};

} // namespace LIIO
