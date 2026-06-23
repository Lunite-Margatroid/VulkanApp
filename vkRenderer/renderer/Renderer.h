#pragma once
#include "Pipeline.h"
#include "VertexBuffer.h"

namespace LT {
	class Renderer {
	private:
		Pipeline m_pipeline;
		std::unique_ptr<VertexBuffer> m_pDebugVertexBuffer;
	public:
		Renderer();
		~Renderer();

		void DrawFrame();
	};
} // namespace LT