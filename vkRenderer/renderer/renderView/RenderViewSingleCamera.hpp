// 渲染视图 单相机
#pragma once
#include "RenderView.hpp"

namespace LT {

	class ConstBuffer;

	class RenderViewSingleCamera : public RenderView
	{
	private:

		// std130
		struct MVPMatrixBuffer {
			glm::mat4 modelMat;
			glm::mat4 viewMat;
			glm::mat4 projectionMat;
			glm::mat4 mvpMat;
			// 列主序
			// 第四行无用 内存对齐 std130
			// 实际是一个3x3矩阵
			glm::vec4 normalMatCol[3];
			// cameraPos.w 无用 内存对齐
			glm::vec4 cameraPos;
		} m_MVPMatBuf;

		ConstBuffer* m_pConstBuffer;
		

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
	};
} // namespace LT
