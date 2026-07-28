#pragma once

#include "ImageViewable.h"

namespace LT {
	class Image2DShaderRes : public ImageViewable {
		friend class ImageManager;
	protected:
		Image2DShaderRes(ImageID id, vk::Format eFormat, uint32_t width, uint32_t height);
		~Image2DShaderRes();

		Image2DShaderRes(const Image2DShaderRes&) = delete;
		Image2DShaderRes(Image2DShaderRes&&) = delete;

		Image2DShaderRes& operator = (const Image2DShaderRes&) = delete;
		Image2DShaderRes& operator = (Image2DShaderRes&&) = delete;

		void InitVKImage() override;


	public:

		/// <summary>
		/// 分配空间并赋值 请保证pData有效，且格式与Image相符
		/// 并初始化ImageView
		/// </summary>
		/// <param name="pData"></param>
		void AssignMemory( const void* pData, size_t nSize) override;
	};
} // namespace LT