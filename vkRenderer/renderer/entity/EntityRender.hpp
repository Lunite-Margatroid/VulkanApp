// 渲染器实体
#pragma once
#include "IEntity.hpp"
#include "IRenderStage.hpp"
#include "RenderView.hpp"

namespace LT {

	struct EntityDrawInfo {
		uint32_t nWidth;
		uint32_t nHeight;
		FlightFrameIndex nFlightFrameIndex;
		RenderStageType eRenderStage;
		ImageID nDepthBuffer;
		RenderView* pRenderView;
		std::vector<ImageID> vecRenderTargets;
		
		std::vector<vk::Semaphore> vecSemWait;
		std::vector<vk::PipelineStageFlags> vecSemWaitMasks;

		std::vector<vk::Semaphore> vecSemSignal;

		vk::Fence vkFenceSet;

		glm::mat4 matModule;

		EntityDrawInfo() 
			: nWidth(0)
			, nHeight(0)
			, nFlightFrameIndex(-1)
			, eRenderStage(RenderStageType::eUnknown)
			, nDepthBuffer(INVALID_IMAGE_ID)
			, pRenderView(nullptr)
			, matModule(glm::identity<glm::mat4>())
		{
			
		}

	};

	class EntityRender : public IEntity {
	public:
		EntityRender(EntityID);
		~EntityRender() = default;

		EntityRender(EntityRender&&) = delete;
		EntityRender(const EntityRender&) = delete;
		EntityRender& operator = (EntityRender&&) = delete;
		EntityRender& operator = (const EntityRender&) = delete;

		virtual void Draw(const EntityDrawInfo& sDrawInfo) = 0;

		virtual void UpdateTransBuffer(const EntityDrawInfo& sDrawInfo) = 0;
	};


} // namespace LT