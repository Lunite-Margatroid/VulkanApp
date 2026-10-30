#include "LunarImageIO.hpp"
#include "OpenImageIO/imageio.h"

#include "logger.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <vector>

namespace LIIO {

    OIIO::TypeDesc GetTypeDesc(ImageFormat eFormat) {
        int nBytesPerChannel = GetBytesPerChannel(eFormat);
        bool bIsFloatFormat = IsFloatFormat(eFormat);
        if (nBytesPerChannel == 1)
        {
            return OIIO::TypeDesc::UINT8;
        }
        else if (nBytesPerChannel == 2 && bIsFloatFormat)
        {
            return OIIO::TypeDesc::HALF;
        }
        else if (nBytesPerChannel == 2 && !bIsFloatFormat)
        {
            return OIIO::TypeDesc::UINT16;
        }
        else if(nBytesPerChannel == 4 && bIsFloatFormat)
        {
            return OIIO::TypeDesc::FLOAT;
        }

        return OIIO::TypeDesc::UNKNOWN;

    }

    ImgRes::ImgRes(const char* path, int nDepth) :
        m_Format(static_cast<ImageFormat>(0)),
        m_Width(0),
        m_Height(0),
        m_Data(nullptr)
    {
        //bool bDone = attribute("debug", 2);
        //bDone = attribute("imagebuf:print_uncaught_errors", 1);
        //bDone = attribute("oiio:print_uncaught_errors", 1);

        if (nDepth != 8 && nDepth != 32 && nDepth != 16) {
            LOG_ERROR("Unsupported Depth\n");
            return;
        }


        if (!std::filesystem::exists(path)) {
            LOG_ERROR("File does not exist. %s\n", path);
            return;
        }

        std::unique_ptr<OIIO::ImageInput> inputImage;
        

        inputImage = OIIO::ImageInput::open(path);

        if (!inputImage)
        {
            std::string err = OIIO::geterror();
			LOG_ERROR("Cannt open file:%s. OIIO Error: %s\n", path, err.c_str());
        }

        const OIIO::ImageSpec& pSpec = inputImage->spec();

		m_Width = pSpec.width;
        m_Height = pSpec.height;
		// 目前全都转成了4通道, 由 m_Format 的 RGBA 格式表示

        OIIO::TypeDesc type = OIIO::TypeDesc(OIIO::TypeDesc::UINT8);

        switch (nDepth)
        {
		case 8:
			m_Format = ImageFormat::RGBA8;
			type = OIIO::TypeDesc(OIIO::TypeDesc::UINT8);
			break;
        case 16:
			// OIIO 的 HALF 为 16 位浮点
			m_Format = ImageFormat::RGBA16_F;
			type = OIIO::TypeDesc(OIIO::TypeDesc::HALF);
            break;
		case 32:
			m_Format = ImageFormat::RGBA32_F;
			type = OIIO::TypeDesc(OIIO::TypeDesc::FLOAT);
			break;
        default:
            break;
        }

		const int nChannels = GetChannelCount(m_Format);
		m_Data = new unsigned char[static_cast<std::size_t>(m_Width) * m_Height * GetBytesPerPixel(m_Format)];

		inputImage->read_image(0, 0, 0, nChannels, type, m_Data, OIIO::AutoStride, OIIO::AutoStride, OIIO::AutoStride);

        if (inputImage->has_error())
        {
			std::string err = inputImage->geterror();
			LOG_ERROR("Cannt read file:%s. OIIO Error: %s\n", path, err.c_str());
        }

    }

    ImgRes::ImgRes(const unsigned char* data, int nWidth, int nHeight, ImageFormat eFormat)
    {
        m_Format = eFormat;
        m_Width = nWidth;
        m_Height = nHeight;
        size_t nSize = nWidth * nHeight * GetBytesPerPixel(eFormat);
        if (nSize > 0)
        {
            m_Data = new unsigned char[nSize];
            memcpy(m_Data, data, nSize);
        }
        else
        {
            m_Data = nullptr;
        }
    }

    // 用纯色初始化指定格式的图像
    ImgRes::ImgRes(int nWidth, int nHeight, ImageFormat eFormat, const AColor& color)
        : m_Format(static_cast<ImageFormat>(0))
        , m_Width(0)
        , m_Height(0)
        , m_Data(nullptr)
    {
        if (nWidth <= 0 || nHeight <= 0 || !IsValidImageFormat(eFormat))
        {
            LOG_ERROR("ImgRes: invalid size or ImageFormat\n");
            return;
        }

        const int nChannels = GetChannelCount(eFormat);
        const OIIO::TypeDesc eType = GetTypeDesc(eFormat);
        if (nChannels < 1 || nChannels > 4 || eType == OIIO::TypeDesc::UNKNOWN)
        {
            LOG_ERROR("ImgRes: unsupported ImageFormat\n");
            return;
        }

        m_Format = eFormat;
        m_Width = nWidth;
        m_Height = nHeight;

        const int nBytesPerPixel = GetBytesPerPixel(eFormat);
        const std::size_t nRowBytes = static_cast<std::size_t>(m_Width) * nBytesPerPixel;

        // 一行像素的 float 源数据 (不足 4 通道时取前 nChannels 个分量)
        const float afSrc[4] = { color.color.r, color.color.g, color.color.b, color.alpha };
        std::vector<float> vecRowSrc(static_cast<std::size_t>(m_Width) * nChannels);
        for (int x = 0; x < m_Width; ++x)
        {
            for (int c = 0; c < nChannels; ++c)
            {
                vecRowSrc[static_cast<std::size_t>(x) * nChannels + c] = afSrc[c];
            }
        }

        m_Data = new unsigned char[static_cast<std::size_t>(m_Width) * m_Height * nBytesPerPixel];

        // 转换到目标格式并写入首行 (支持 half), 其余行直接拷贝
        if (!OIIO::convert_pixel_values(OIIO::TypeDesc::FLOAT, vecRowSrc.data(),
                                        eType, m_Data, m_Width * nChannels))
        {
            LOG_ERROR("ImgRes: convert fill color failed\n");
            delete[] m_Data;
            m_Data = nullptr;
            m_Format = static_cast<ImageFormat>(0);
            m_Width = 0;
            m_Height = 0;
            return;
        }

        for (int y = 1; y < m_Height; ++y)
        {
            std::memcpy(m_Data + static_cast<std::size_t>(y) * nRowBytes, m_Data, nRowBytes);
        }
    }

    ImgRes::ImgRes()
        : m_Format(static_cast<ImageFormat>(0))
        , m_Width(0)
        , m_Height(0)
        , m_Data(nullptr)
    {
    }

    // 移动构造: 转移数据所有权, 源对象置为空状态
    ImgRes::ImgRes(ImgRes&& other) noexcept
        : m_Format(other.m_Format)
        , m_Width(other.m_Width)
        , m_Height(other.m_Height)
        , m_Data(other.m_Data)
    {
        other.m_Format = static_cast<ImageFormat>(0);
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Data = nullptr;
    }

    // 移动赋值: 释放自身数据后接管源对象的数据, 源对象置为空状态
    ImgRes& ImgRes::operator = (ImgRes&& other) noexcept
    {
        if (this != &other)
        {
            Release();

            m_Format = other.m_Format;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Data = other.m_Data;

            other.m_Format = static_cast<ImageFormat>(0);
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_Data = nullptr;
        }
        return *this;
    }

    ImgRes::~ImgRes() {
        Release();
    }

    void ImgRes::Release() {
        if (m_Data)
        {
            delete[] m_Data;
            m_Data = nullptr;

            m_Height = 0;
            m_Width = 0;
            m_Format = static_cast<ImageFormat>(0);
        }
    }

    bool ImgRes::Resize(int newSizeWidth, int newSizeHeight)
    {
        // TODO: 处理Resize
        return false;
    }
    void ImgRes::SavaImage(const char* strPath, ImageFormat eFormat/* = ImageFormat::RGBA8*/ ) const
    {
        auto imageOutput = OIIO::ImageOutput::create(strPath);

        if (!imageOutput)
        {
            LOG_WARNING("SavePng: Unsupported Format %s", strPath);
            return;
        }

        

        OIIO::TypeDesc eTypeDesc = GetTypeDesc(m_Format);

        OIIO::ImageSpec imageSpec(m_Width, m_Height, GetChannelCount(m_Format), eTypeDesc);


        // 打开文件准备写入
        if (!imageOutput->open(strPath, imageSpec)) {
            LOG_WARNING("SavePng: Can't open file: %s", strPath);
            return;
        }

        OIIO::TypeDesc eSaveType = GetTypeDesc(eFormat);

        imageOutput->write_image(eSaveType, m_Data);

        imageOutput->close();
    }

    void ImgRes::CopyRegion(const ImgRes& other, int distX, int distY, int srcX, int srcY, int nWidth, int nHeight) {
        if (other.m_Format != m_Format)
        {
            LOG_ERROR("CopyRegion: ImageFormat mismatch\n");
            return;
        }
        if (!m_Data || !other.m_Data)
        {
            LOG_ERROR("CopyRegion: null image data\n");
            return;
        }
        if (nWidth <= 0 || nHeight <= 0)
        {
            return;
        }
        // 源区域与目标区域都必须完整落在各自图像内
        if (srcX < 0 || srcY < 0
            || srcX + nWidth > other.m_Width || srcY + nHeight > other.m_Height
            || distX < 0 || distY < 0
            || distX + nWidth > m_Width || distY + nHeight > m_Height)
        {
            LOG_ERROR("CopyRegion: region out of bounds\n");
            return;
        }

        const int nBytesPerPixel = GetBytesPerPixel(m_Format);
        const std::size_t nRowBytes = static_cast<std::size_t>(nWidth) * nBytesPerPixel;

        // 同一对象且目标在源下方时, 从末行向前拷贝, 避免覆盖尚未读取的源数据
        const bool bReverse = (this == &other) && (distY > srcY);

        for (int i = 0; i < nHeight; ++i)
        {
            const int y = bReverse ? (nHeight - 1 - i) : i;
            const unsigned char* pSrc = other.m_Data
                + (static_cast<std::size_t>(srcY + y) * other.m_Width + srcX) * nBytesPerPixel;
            unsigned char* pDst = m_Data
                + (static_cast<std::size_t>(distY + y) * m_Width + distX) * nBytesPerPixel;
            // memmove 处理同一行内源/目标重叠的情况
            std::memmove(pDst, pSrc, nRowBytes);
        }
    }

    // 在图像上绘制矩形, 按 alpha 做 source-over 混合
    void ImgRes::DrawRect(int x, int y, int width, int height, const AColor& color)
    {
        if (!m_Data || width <= 0 || height <= 0)
        {
            return;
        }

        const OIIO::TypeDesc eType = GetTypeDesc(m_Format);
        if (eType == OIIO::TypeDesc::UNKNOWN)
        {
            LOG_ERROR("DrawRect: unsupported ImageFormat\n");
            return;
        }

        const float fAlpha = std::clamp(color.alpha, 0.0f, 1.0f);
        if (fAlpha <= 0.0f)
        {
            return;
        }

        // 裁剪到图像范围
        const int nX0 = std::max(x, 0);
        const int nY0 = std::max(y, 0);
        const int nX1 = std::min(x + width, m_Width);
        const int nY1 = std::min(y + height, m_Height);
        if (nX0 >= nX1 || nY0 >= nY1)
        {
            return;
        }

        const int nChannels = GetChannelCount(m_Format);
        const int nBytesPerPixel = GetBytesPerPixel(m_Format);
        const int nRectWidth = nX1 - nX0;
        const std::size_t nValueCount = static_cast<std::size_t>(nRectWidth) * nChannels;

        // 源颜色; 4 通道时 alpha 参与混合
        const float afSrc[4] = { color.color.r, color.color.g, color.color.b, fAlpha };

        std::vector<float> vecRow(nValueCount);

        for (int iy = nY0; iy < nY1; ++iy)
        {
            unsigned char* pRow = m_Data
                + (static_cast<std::size_t>(iy) * m_Width + nX0) * nBytesPerPixel;

            // 目标行 -> float (归一化值)
            if (!OIIO::convert_pixel_values(eType, pRow, OIIO::TypeDesc::FLOAT,
                                            vecRow.data(), static_cast<int>(nValueCount)))
            {
                LOG_ERROR("DrawRect: convert to float failed\n");
                return;
            }

            for (std::size_t i = 0; i < nValueCount; ++i)
            {
                const float fDst = vecRow[i];
                const int c = static_cast<int>(i % nChannels);
                if (c == 3)
                {
                    // alpha 通道: out.a = src.a + dst.a * (1 - src.a)
                    vecRow[i] = fAlpha + fDst * (1.0f - fAlpha);
                }
                else
                {
                    // 颜色通道: out.rgb = src.rgb * src.a + dst.rgb * (1 - src.a)
                    vecRow[i] = afSrc[c] * fAlpha + fDst * (1.0f - fAlpha);
                }
            }

            // float -> 目标格式
            if (!OIIO::convert_pixel_values(OIIO::TypeDesc::FLOAT, vecRow.data(), eType,
                                            pRow, static_cast<int>(nValueCount)))
            {
                LOG_ERROR("DrawRect: convert back failed\n");
                return;
            }
        }
    }
} // namespace LIIO