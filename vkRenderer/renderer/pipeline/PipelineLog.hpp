#pragma once
#include "Pipeline.hpp"
#include "LunarTextIO.hpp"
#include "TextTable.hpp"

namespace LT {
	class PipelineLog : public Pipeline {
	protected:

	public:
		PipelineLog();
		void LogPipelineInfo();
	};
}