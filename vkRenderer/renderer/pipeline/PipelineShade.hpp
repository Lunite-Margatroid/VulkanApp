#include "Pipeline.hpp"
#include "RenderStageOpaqueForward.hpp"
namespace LT {
	class PipelineShade : public Pipeline{
	protected:

		std::vector<Image2DDepthBuffer*> m_vecDepthBuffer; // 深度缓冲
		RenderStageOpaqueForward m_cStage;
		std::shared_ptr<LightManager> m_pLightManager;
	public:
		PipelineShade();
		~PipelineShade();

		virtual void Resize(uint32_t nWidth, uint32_t nHeight);

		virtual void Execute(FrameInfo& sInOutFrameInfo);

		DescriptorSetWrapper* GetLightBufferDescriptorSet();
		ConstBufferHandle GetLightBufferHandle();

		virtual void SetColorInType(const std::vector<PipelineIOType>& vecColorInType) override;
		virtual void SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType) override;
		virtual void SetDepthInType(PipelineIOType eDepthInType) override;
		virtual void SetDepthOutType(PipelineIOType eDepthOutType) override;
	};

} // namespace