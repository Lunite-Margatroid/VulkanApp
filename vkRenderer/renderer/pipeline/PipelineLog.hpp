#pragma once
#include "Pipeline.hpp"
#include "LunarTextIO.hpp"
#include "TextTable.hpp"
#include "RenderStageLog.hpp"

namespace LT {

	class EntityRender;

	class PipelineLog : public Pipeline {
	protected:
		RenderStageLog m_stageLog;
		std::array<ConstBufferHandle, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrTransBuffer;
		VPMatrixBuffer2D m_sTransBuffer;
		std::vector<EntityRender*> m_vecLogLine;

	public:
		PipelineLog();
		~PipelineLog();
		void LogPipelineInfo();

		void Resize(uint32_t nWidth, uint32_t nHeight) override;
		void Execute(PipelineExecuteInfo& sInOutPipelineInfo) override;

		void SetColorInType(const std::vector<PipelineIOType>& vecColorInType) override;
		void SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType) override;
		void SetDepthInType(PipelineIOType eDepthInType) override;
		void SetDepthOutType(PipelineIOType eDepthOutType) override;
	};
}