#include "DeviceImage.h"

namespace LT {
	class Image2DShaderRes : public DeviceImage{
	protected:
		Image2DShaderRes(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height);
		~Image2DShaderRes();

		Image2DShaderRes(const Image2DShaderRes&) = delete;
		Image2DShaderRes(Image2DShaderRes&&) = delete;

		Image2DShaderRes& operator = (const Image2DShaderRes&) = delete;
		Image2DShaderRes& operator = (Image2DShaderRes&&) = delete;

		void InitVKImage() override;

	public:
	};
} // namespace LT