#pragma once
#include "Pipeline.h"
#include "VertexBuffer.h"


namespace LT {
	class Renderer {
	private:
		std::unique_ptr<Pipeline> m_pPipeline;
		VertexBuffer* m_pDebugVertexBuffer;
		IndexBuffer* m_pDebugIndexBuffer;

		struct MVPMatrixBuffer {
			glm::mat4 modelMat;
			glm::mat4 viewMat;
			glm::mat4 projectionMat;
		} m_MVPMatBuf;


		// 由当前实例管理
		std::vector<ConstBuffer*> m_vecConstBufferMVPMat; // 数量与flight frame一致

	public:
		Renderer();
		~Renderer();

		void SetModelMat(const float* pModelMat);
		void SetViewMat(const float* pViewMat);
		void SetProjectionMat(const float* pProjectionMat);

		void UpdateConstBufer();

		void DrawFrame();
	};
} // namespace LT