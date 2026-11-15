#pragma once
#include "IRenderStage.hpp"
#include "LogPass.hpp"

class EntityRenderLogLine;

namespace LT {
	class RenderStageLog : public IRenderStage {

	public:
		RenderStageLog();
		~RenderStageLog();


		void Execute(StageExecuteInfo& sInOutExecuteInfo) override;
		
	};


}; // namespace LT