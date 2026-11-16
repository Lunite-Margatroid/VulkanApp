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

		void Resize(uint32_t nWidth, uint32_t nHeight) override;

		void Execute(PipelineExecuteInfo& sInOutPipelineInfo) override;

		DescriptorSetWrapper* GetLightBufferDescriptorSet();
		ConstBufferHandle GetLightBufferHandle();

		void SetColorInType(const std::vector<PipelineIOType>& vecColorInType) override;
		void SetColorOutType(const std::vector<PipelineIOType>& vecColorOutType) override;
		void SetDepthInType(PipelineIOType eDepthInType) override;
		void SetDepthOutType(PipelineIOType eDepthOutType) override;
	};

} // namespace