#pragma once
#include "IRenderStage.hpp"
#include "LogPass.hpp"
#include "Camera2D.hpp"


namespace LT {
	class ConstBuffer;
	class EntityRenderLogLine;

	// Note.目前仅允许创建一个实例
	class RenderStageLog : public IRenderStage {

		LogPass m_pass;

		// descriptorSetlayout
		// trans buffer
		vk::DescriptorSetLayout m_vk2DTransBufferDescSetLayout;

		// DescritporSet
		std::array<DescriptorSetWrapper, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrDescSet;

		std::array<ConstBuffer*, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrTransBuffer2D;

		VPMatrixBuffer2D m_sVPMat2D;

		Camera2D m_camera;

		// 所有权不在该对象
		ImageID m_nASCIITex;

		bool m_bVPMat2DDirt;

	public:
		RenderStageLog();
		~RenderStageLog();

		void Execute(StageExecuteInfo& sInOutExecuteInfo) override;
		
		RenderStageType GetRenderStageType() const override;

		void SetASCIITex(ImageID nASCIITex);

		void UpdateCamera(uint32_t nWidth, uint32_t nHeight);
	};


}; // namespace LT