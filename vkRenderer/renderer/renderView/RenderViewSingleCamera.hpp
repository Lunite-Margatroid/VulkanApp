// 渲染视图 单相机
#pragma once
#include "RenderView.hpp"
#include "EngineCommon.h" 

namespace LT {

	class ConstBuffer;

	class RenderViewSingleCamera : public RenderView
	{
	private:
		// std130
		MVPMatrixBuffer m_MVPMatBuf;
	public:
		RenderViewSingleCamera();
		~RenderViewSingleCamera();

		// 五法则:删除拷贝和移动
		RenderViewSingleCamera(const RenderViewSingleCamera&) = delete;
		RenderViewSingleCamera& operator=(const RenderViewSingleCamera&) = delete;
		RenderViewSingleCamera(RenderViewSingleCamera&&) = delete;
		RenderViewSingleCamera& operator=(RenderViewSingleCamera&&) = delete;

		void SetModelMat(const glm::mat4& matModel);
		void SetProjectionMat(const glm::mat4& matProjection);
		void SetViewMat(const glm::mat4& matView);
		void SetCameraPos(const glm::vec3& vec3CameraPos);

		const MVPMatrixBuffer* GetTransBuffer();
	};
} // namespace LT
