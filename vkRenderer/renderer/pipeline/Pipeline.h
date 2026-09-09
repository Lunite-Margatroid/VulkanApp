#pragma once
// material
#include "MaterialMainTexture.hpp"
#include "MaterialManager.hpp"
// Mesh
#include "MeshManager.hpp"

// buffer
#include "ConstBuffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "BufferManager.h"

// Image
#include "Image2DDepthBuffer.h"
#include "Image2DShaderRes.h"
#include "ImageManager.h"

// Entity
#include "EntityRenderMesh.hpp"

// Render View
#include "RenderViewSingleCamera.hpp"

// RenderPass
#include "GraphicPass.hpp"

// Render Stage
#include "RenderStageOpaqueForward.hpp"

namespace LT {

	class VertexBuffer;
	class IndexBuffer;
	class ConstBuffer;
	class ImageSampler;
	class Image2DShaderRes;
	class Image2DDepthBuffer;
	class GraphicPass;
	class EntityRender; 
	class RenderViewSingleCamera;

	struct FrameInfo {
		// 只增的帧序列
		FrameIndex nFrameIndex;
		// Flight Frame的索引
		// 小于最大Flight Frame的数量
		FlightFrameIndex nFightFrameIndex;
		// 渲染对象
		std::vector<EntityRender*> vecEntityRender;
		// RenderView
		RenderViewSingleCamera* pRenderView;

		std::vector<ImageID> vecRenderTargets;


		// acquire发出的信号
		vk::Semaphore semAcquiring;

		// 渲染完成
		vk::Semaphore semDrawing;
		vk::Fence fenceDrawing;

	};


	class Pipeline {
	protected:
		// render target size
		uint32_t m_nWidth;
		uint32_t m_nHeight;

		std::vector<Image2DDepthBuffer*> m_vecDepthBuffer; // 深度缓冲

	public:
		Pipeline();
		~Pipeline();
		void DrawFrame();


		void UpdateConstBuffer();

		void Resize(uint32_t nWidth, uint32_t nHeight);

		void Execute(const FrameInfo& sFrameInfo);


		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }
	};
}// namespace LT