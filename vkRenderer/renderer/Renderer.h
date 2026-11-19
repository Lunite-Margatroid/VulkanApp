#pragma once

#include "Pipeline.hpp"


namespace LT {
	class View;
	class DisplayDevice;
	class Node;

	constexpr uint32_t RENDERER_SEMPHORE_COUNT = 2u;

	class Renderer {
	private:
		// 由当前实例管理
		std::unique_ptr<RenderViewSingleCamera> m_pRenderView;

		DisplayDevice* m_pDisplayDevice;
		View* m_pView;

		// 在不同Pipeline之间使用的信号量
		std::array<std::array<vk::Semaphore, RENDERER_SEMPHORE_COUNT>, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrSemphores;

	public:
		Renderer();
		~Renderer();

		void SetView(View* pView);
		void SetDisplayDevice(DisplayDevice* pDisplayDevice);

		void SetViewMat(const glm::mat4& matView);
		void SetProjectionMat(const glm::mat4& matProjection);
		void SetCameraPos(const glm::vec4& vec4CameraPos);

		void DrawFrame(FrameInfo& sInOutFrameInfo);
	};
} // namespace LT