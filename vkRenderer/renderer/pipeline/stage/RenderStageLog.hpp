#pragma once
#include "IRenderStage.hpp"
#include "LogPass.hpp"

class EntityRenderLogLine;

namespace LT {
	// Note.目前仅允许创建一个实例
	class RenderStageLog : public IRenderStage {

		LogPass m_pass;

		// descriptorSetlayout
		// trans buffer
		vk::DescriptorSetLayout m_vk2DTransBufferDescSetLayout;

		// DescritporSet
		std::array<DescriptorSetWrapper, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrDescSet;

	public:
		RenderStageLog();
		~RenderStageLog();

		void Execute(StageExecuteInfo& sInOutExecuteInfo) override;
		
		RenderStageType GetRenderStageType() const override;
	};


}; // namespace LT