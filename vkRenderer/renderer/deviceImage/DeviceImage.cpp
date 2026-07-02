#include "vkRendererCommon.h"
#include "vkContext.h"
#include "DeviceImage.h"

namespace LT {
    DeviceImage::DeviceImage(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height)
        : m_nID(id)
        , m_nWidth(width)
        , m_nHeight(height)
        , m_eFormat(eFormat)
    {

    }
    DeviceImage::~DeviceImage()
    {
        vkContext::GetVkDevice().destroyImage(m_vkImage);
    }
} // namespace LT