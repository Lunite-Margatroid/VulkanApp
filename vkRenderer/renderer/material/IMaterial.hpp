// 材质基类
#pragma once
#include "IRenderStage.hpp"
#include "RenderPass.hpp"

namespace LT {
	struct MaterialSlot {
		vk::DescriptorType eDescType;
		int64_t nSrcID;
	};


	class IMaterial {
		friend class MaterialManager;
	protected:
		MaterialID m_nID;
		std::unordered_map<BindingInfo, MaterialSlot, BindingInfoHash> m_mapSlots;

	protected:
		IMaterial(MaterialID nID);

		virtual void RegisterStage(RenderStageType eStage) = 0;
	public:
		using _RenderPassMap = std::map<RenderPassFlag, RenderPass*>;
		using RenderPassMap = std::map<RenderStageType, _RenderPassMap>;

		virtual ~IMaterial();

		virtual RenderPass* GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag) = 0;
		virtual void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag, FlightFrameIndex nFlightFrameIndex) = 0;


		ResultSetter SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID);
	};

	template<typename DerivedMaterial>
	class BaseMaterial : public IMaterial
	{
		friend class MaterialManager;
	protected:
		static RenderPassMap s_mapRenderPasses;

	protected:
		BaseMaterial(MaterialID nID):IMaterial(nID)
		{
		}

		void RegisterStage(RenderStageType eStage) override
		{
			if (m_mapRenderPasses.find(eStage) == m_mapRenderPasses.end())
			{
				m_mapRenderPasses[eStage] = _RenderPassMap();
			}
		}
	};

	template<typename DerivedMaterial>
	IMaterial::RenderPassMap BaseMaterial<DerivedMaterial>::s_mapRenderPasses;
} // namespace LT