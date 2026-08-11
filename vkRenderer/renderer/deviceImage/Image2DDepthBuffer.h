#pragma once
#include "ImageViewable.h"

namespace LT {
	class Image2DDepthBuffer : public ImageViewable{
		friend class ImageManager;

	protected:
		Image2DDepthBuffer(ImageID id, uint32_t width, uint32_t height);

		Image2DDepthBuffer(const Image2DDepthBuffer& ) = delete;
		Image2DDepthBuffer(Image2DDepthBuffer&&) = delete;

		Image2DDepthBuffer& operator = (const Image2DDepthBuffer&) = delete;
		Image2DDepthBuffer& operator = (Image2DDepthBuffer&&) = delete;

		~Image2DDepthBuffer();


		void InitVKImage() override;

	public:
		/// <summary>
		/// 初始化内存并创建ImageView
		/// 该对象不支持写入 参数被忽略
		/// </summary>
		/// <param name="pData"> 略 </param>
		/// <param name="nSize"> 略 </param>
		void AssignMemory(const void* pData, size_t nSize) override;

	};
} // namespace LT