// 材质基类
#pragma once
#include "IRenderStage.hpp"
#include "RenderPass.hpp"

namespace LT {
	using MaterialID = int64_t;
	constexpr MaterialID INVALID_MATERIAL_ID = INVALID_ITEM_ID;

	enum class MaterialType {
		eUndifined = -1,
		eMainTexture
	};


	struct MaterialSlot {
		vk::DescriptorType eDescType;
		int64_t nSrcID;
	};


	class IMaterial {
	protected:
		using RenderPassMap = std::map<RenderPassFlag, RenderPass*>;
	protected:
		MaterialID m_nID;
		std::map<RenderStageType, RenderPassMap> m_mapRenderPasses;
		std::unordered_map<BindingInfo, MaterialSlot, BindingInfoHash> m_mapSlots;

	protected:
		IMaterial(MaterialID nID);
		~IMaterial();

		void RegisterStage(RenderStageType eStage);
	public:
		virtual RenderPass* GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag) = 0;
		virtual void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag, FlightFrameIndex nFlightFrameIndex) = 0;

		ResultSetter SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID);
	};
} // namespace LT