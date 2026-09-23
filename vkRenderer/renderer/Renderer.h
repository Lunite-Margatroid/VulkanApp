#pragma once

#include "Pipeline.h"


namespace LT {
	class View;
	class DisplayDevice;
	class Node;

	class Renderer {
	private:
		std::unique_ptr<Pipeline> m_pPipeline;
		VertexBuffer* m_pDebugVertexBuffer;
		IndexBuffer* m_pDebugIndexBuffer;
		Image2DShaderRes* m_pDebugImage;
		std::unique_ptr<EntityRenderMesh> m_pEntity;
		MaterialRef m_refMtl;

		struct MVPMatrixBuffer {
			glm::mat4 modelMat;
			glm::mat4 viewMat;
			glm::mat4 projectionMat;
			glm::mat4 mvpMat;
			glm::vec4 normalMatCol[3];
			glm::vec4 cameraPos;
		} m_MVPMatBuf;


		// 由当前实例管理
		std::unique_ptr<RenderViewSingleCamera> m_pRenderView;

		std::vector<vk::Semaphore> m_vecSemDrawing;
		std::vector<vk::Semaphore> m_vecSemAcquiring;
		std::vector<vk::Fence> m_vecFenceDrawing;

		DisplayDevice* m_pDisplayDevice;
		View* m_pView;


	public:
		Renderer();
		~Renderer();

		void SetView(View* pView);
		void SetDisplayDevice(DisplayDevice* pDisplayDevice);

		void SetModelMat(const glm::mat4& matModel);
		void SetViewMat(const glm::mat4& matView);
		void SetProjectionMat(const glm::mat4& matProjection);
		void SetCameraPos(const glm::vec4& vec4CameraPos);

		void Resize(uint32_t nWidth, uint32_t nHeight);

		void DrawFrame(const FrameInfo& sFrameInfo);
	};
} // namespace LT