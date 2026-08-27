// 渲染对象
#pragma once

#include "ImageViewable.h"

namespace LT {
	class Image2DColorAttachment : public ImageViewable{
		friend class ImageManager;
	protected:
		Image2DColorAttachment(ImageID nID, vk::Format eFormat, uint32_t nWidth, uint32_t nHeight);
		~Image2DColorAttachment();

		Image2DColorAttachment(const Image2DColorAttachment&) = delete;
		Image2DColorAttachment(Image2DColorAttachment&&) = delete;

		Image2DColorAttachment& operator = (const Image2DColorAttachment&) = delete;
		Image2DColorAttachment& operator = (Image2DColorAttachment&&) = delete;

		void InitVKImage() override;

	public:
		/// <summary>
		/// 不设置初始缓冲 参数无用
		/// 创建ImageView
		/// </summary>
		/// <param name="pData"></param>
		/// <param name="nSize"></param>
		void AssignMemory(const void* pData, size_t nSize) override;
	};

} // namespace LT