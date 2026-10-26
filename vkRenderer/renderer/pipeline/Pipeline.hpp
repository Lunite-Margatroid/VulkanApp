#pragma once
// material
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
#include "IRenderStage.hpp"

// Fence Manager
#include "FenceManager.hpp"

// Light Manager
#include "LightManager.hpp"

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

		// 当前帧最后一次Submit的Fence
		// 通常作为输出参数
		FenceRef fenceDrawing;

		uint32_t nWidth;
		uint32_t nHeight;

		bool bLightDirty; // 该帧灯光是否有更新
		std::vector<std::pair<LightUniformBuffer, std::bitset<LIGHT_GROUP_MAX_COUNT>>> vecLightBuffer;

	};


	class Pipeline {
	protected:
		// render target size
		uint32_t m_nWidth;
		uint32_t m_nHeight;

		std::vector<PipelineIOType> m_vecColorInType;
		std::vector<PipelineIOType> m_vecColorOutType;
		PipelineIOType m_eDepthInType;
		PipelineIOType m_eDepthOutType;
	public:
		Pipeline();
		virtual ~Pipeline();

		void UpdateConstBuffer();

		virtual void Resize(uint32_t nWidth, uint32_t nHeight) = 0;

		virtual void Execute(FrameInfo& sInOutFrameInfo) = 0;

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

		virtual void SetColorInType(const std::vector<PipelineIOType>& vecColorInType);
		virtual void SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType);

		virtual void SetDepthInType(PipelineIOType eDepthInType);
		virtual void SetDepthOutType(PipelineIOType eDepthOutType);
	};
}// namespace LT