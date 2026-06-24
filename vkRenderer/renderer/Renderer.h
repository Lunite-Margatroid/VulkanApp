#pragma once
#include "Pipeline.h"
#include "VertexBuffer.h"

namespace LT {
	class Renderer {
	private:
		std::unique_ptr<Pipeline> m_pPipeline;
		std::unique_ptr<VertexBuffer> m_pDebugVertexBuffer;
	public:
		Renderer();
		~Renderer();

		void DrawFrame();
	};
} // namespace LT