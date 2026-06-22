#pragma once
#include "Pipeline.h"

namespace LT {
	class Renderer {
	private:
		Pipeline m_pipeline;
	public:
		Renderer();
		~Renderer();

		void DrawFrame();
	};
} // namespace LT