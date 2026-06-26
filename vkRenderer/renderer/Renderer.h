#pragma once
#include "Pipeline.h"
#include "VertexBuffer.h"

namespace LT {
	class Renderer {
	private:
		std::unique_ptr<Pipeline> m_pPipeline;
		VertexBuffer* m_pDebugVertexBuffer;
		IndexBuffer* m_pDebugIndexBuffer;
	public:
		Renderer();
		~Renderer();

		void DrawFrame();
	};
} // namespace LT