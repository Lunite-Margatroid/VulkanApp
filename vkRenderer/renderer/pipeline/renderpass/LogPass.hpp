#pragma once
#include "GraphicPass.hpp"
#include "DynamicVertexBuffer.hpp"
#include "LunarTextIO.hpp"

namespace LT {
	class LogPass : public GraphicPass {
		friend class RenderStageLog;
	protected:


	private:
		LogPass();
		~LogPass();
		
	};

}// namespace LT