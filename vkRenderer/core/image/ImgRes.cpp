#include "EngineCommon.h"

#include "OpenImageIO/imageio.h"
#include "ImgRes.h"

using namespace OIIO_CURRENT_NAMESPACE;


namespace LT {

    ImgRes::ImgRes(const std::string& path, int nDepth) :
        m_Data(nullptr),
        m_Height(0),
        m_Width(0),
        m_Channals(0),
        m_Depth(nDepth),
        m_ResultCode(-1)
    {
        //bool bDone = attribute("debug", 2);
        //bDone = attribute("imagebuf:print_uncaught_errors", 1);
        //bDone = attribute("oiio:print_uncaught_errors", 1);

        if (nDepth != 8 && nDepth != 32) {
            LOG_ERROR("Unsupported Depth\n");
            return;
        }


        if (!std::filesystem::exists(path)) {
            LOG_ERROR("File does not exist. %s\n", path.c_str());
            return;
        }

        ImageSpec pSpec;

        bool hasError = has_error();
        // auto input = ImageInput::open(path, &pSpec);

        std::unique_ptr<ImageInput> input = ImageInput::create(path, true);
        // ImageBuf input(path);

        if (input) {
            if (input->has_error())
            {
                std::string oiioErr = input->geterror();
                LOG_ERROR("Cannt open file:%s. OIIO Error: %s\n", path.c_str(), oiioErr.c_str());
                return;
            }
        }
        else
        {
            std::string oiioErr = geterror();
            if (oiioErr.length() > 0 && oiioErr[0]) {
                LOG_ERROR("Cannt open file:%s. OIIO Error: %s\n", path.c_str(), oiioErr.c_str());
            }
            else
            {
                LOG_ERROR("Cannt open file:%s.\n", path.c_str());
            }
            return;
        }
        const ImageSpec& spec = input->spec();

        m_Height = spec.height;
        m_Width = spec.width;
        m_Channals = spec.nchannels;

        TypeDesc type = m_Depth == 32 ? TypeDesc(TypeDesc::FLOAT) : TypeDesc(TypeDesc::UINT8);

        m_Data = new unsigned char[m_Height * m_Width * 4 * type.size()];

        // 目前全都转成了4通道
        // TODO: 处理13通道的情况
        if (m_Channals != 4)
        {
            m_Channals = 4;
            ImageSpec targetSpec(m_Width, m_Height, 4, type);

            unsigned char* pTempData = new unsigned char[m_Height * m_Width * spec.nchannels * type.size()];
            int count = GetPixelCount();

            // 读取并自动转换格式
            input->read_image(0, 0, 0, -1, type, pTempData, AutoStride, AutoStride, AutoStride);
            if (spec.nchannels == 1) {
                // 1通道
                if (m_Depth == 8) {
                    // 8位
                    unsigned char* pRes = pTempData;
                    RGBA_8* pDist = reinterpret_cast<RGBA_8*>(m_Data);
                    // 8位 1通道转4通道
                    for (int i = 0; i < count; i++)
                    {
                        pDist[i].r = pRes[i];
                        pDist[i].g = pRes[i];
                        pDist[i].b = pRes[i];
                        pDist[i].a = 255;
                    }
                }
                else
                {// 32位
                    float* pRes = reinterpret_cast<float*>(pTempData);
                    RGBA_32* pDist = reinterpret_cast<RGBA_32*>(m_Data);
                    // 32位 1通道转4通道
                    for (int i = 0; i < count; i++)
                    {
                        pDist[i].r = pRes[i];
                        pDist[i].g = pRes[i];
                        pDist[i].b = pRes[i];
                        pDist[i].a = 1.0f;
                    }
                }
            }
            else
            {// 3通道
                if (m_Depth == 8) {
                    // 8位
                    RGB_8* pRes = reinterpret_cast<RGB_8*>(pTempData);
                    RGBA_8* pDist = reinterpret_cast<RGBA_8*>(m_Data);
                    // 8位 3通道转4通道
                    for (int i = 0; i < count; i++)
                    {
                        pDist[i].r = pRes[i].r;
                        pDist[i].g = pRes[i].g;
                        pDist[i].b = pRes[i].b;
                        pDist[i].a = 255;
                    }
                }
                else
                {// 32位
                    RGB_32* pRes = reinterpret_cast<RGB_32*>(pTempData);
                    RGBA_32* pDist = reinterpret_cast<RGBA_32*>(m_Data);
                    // 32位 3通道转4通道
                    for (int i = 0; i < count; i++)
                    {
                        pDist[i].r = pRes[i].r;
                        pDist[i].g = pRes[i].g;
                        pDist[i].b = pRes[i].b;
                        pDist[i].a = 1.0f;
                    }
                }
            }
            delete[] pTempData;
        }
        else {
            // 如果原图是4通道 则直接读取
            input->read_image(0, 0, 0, -1, type, m_Data, AutoStride, AutoStride, AutoStride);
        }
        if (!input->close()) {
            std::string strError = input->geterror();
            LOG_ERROR("OIIO Error: %s\n", strError.c_str());
        }
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
            m_Channals = 0;
        }
    }

    bool ImgRes::Resize(int newSizeWidth, int newSizeHeight)
    {
        // TODO: 处理Resize
        return false;
    }
} // namespace LT