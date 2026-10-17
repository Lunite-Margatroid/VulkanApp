// 渲染器实体
#pragma once
#include "IEntity.hpp"
#include "IRenderStage.hpp"
#include "RenderView.hpp"
#include "MaterialManager.hpp"

namespace LT {

	struct EntityDrawInfo {
		//uint32_t nWidth;
		//uint32_t nHeight;
		//FlightFrameIndex nFlightFrameIndex;
		//RenderStageType eRenderStage;
		//ImageID nDepthBuffer;
		//RenderView* pRenderView;
		//std::vector<ImageID> vecRenderTargets;
		//
		//std::vector<vk::Semaphore> vecSemWait;
		//std::vector<vk::PipelineStageFlags> vecSemWaitMasks;

		//std::vector<vk::Semaphore> vecSemSignal;
		vk::PipelineLayout vkLayout;
		// 最后一次Submit的Fence
		// 通常作为输出参数
		//FenceRef refFence;

		CommandBufferWrapper* pCommandBuffer;

		//bool bClear;

		EntityDrawInfo() 
			//: nWidth(0)
			//, nHeight(0)
			//, nFlightFrameIndex(-1)
			//, eRenderStage(RenderStageType::eUnknown)
			//, nDepthBuffer(INVALID_IMAGE_ID)
			//, pRenderView(nullptr)
			//, refFence(INVALID_FENCE_HANDLE)
			//, bClear(false)
			: pCommandBuffer(nullptr)
			, vkLayout(VK_NULL_HANDLE)
		{
			
		}

	};

	class EntityRender : public IEntity {
	protected:
		glm::mat4 m_matModel;
		ModelNormalMat m_sModelNormalMat;

	public:
		EntityRender(EntityID);
		~EntityRender() = default;

		EntityRender(EntityRender&&) = delete;
		EntityRender(const EntityRender&) = delete;
		EntityRender& operator = (EntityRender&&) = delete;
		EntityRender& operator = (const EntityRender&) = delete;

		virtual void Draw(EntityDrawInfo& sInOutDrawInfo) = 0;

		virtual void UpdateTransBuffer(const EntityDrawInfo& sDrawInfo) = 0;

		void SetModuleMat(const glm::mat4& matMod);

		virtual void SetMaterial(const MaterialRef& refMtl) = 0;
		virtual MaterialRef GetMaterial() const = 0;
		virtual MaterialID GetMaterialID() const = 0;

		virtual void SetRenderPassFlag(RenderPassFlag nFlag) = 0;
		virtual RenderPassFlag GetRenderPassFlag() const = 0;
	};


} // namespace LT