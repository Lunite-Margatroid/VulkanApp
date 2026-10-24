#include "Pipeline.hpp"

namespace LT {
	class PipelineShade : public Pipeline{
	protected:

		std::vector<Image2DDepthBuffer*> m_vecDepthBuffer; // 深度缓冲
		RenderStageOpaqueForward m_sStage;
		std::shared_ptr<LightManager> m_pLightManager;
	public:
		PipelineShade();
		~PipelineShade();

		virtual void Resize(uint32_t nWidth, uint32_t nHeight);

		virtual void Execute(FrameInfo& sInOutFrameInfo);

		DescriptorSetWrapper* GetLightBufferDescriptorSet();
		ConstBufferHandle GetLightBufferHandle();
	};

} // namespace