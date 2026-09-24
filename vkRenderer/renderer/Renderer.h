#pragma once

#include "Pipeline.h"


namespace LT {
	class View;
	class DisplayDevice;
	class Node;

	class Renderer {
	private:
		std::unique_ptr<Pipeline> m_pPipeline;


		// 由当前实例管理
		std::unique_ptr<RenderViewSingleCamera> m_pRenderView;

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

		void DrawFrame(const FrameInfo& sFrameInfo);
	};
} // namespace LT