// class ImageManager
// 位于Renderer层的图像资源管理
#include "DeviceImage.h"

namespace LT {
	class ImageManager {
	

	public:
		static DeviceImage* CreateShaderResourceImage2D(vk::Format eFormat, uint32_t width, uint32_t height);
		static void DeleteImage(DeviceImage* pImage);
	};
}