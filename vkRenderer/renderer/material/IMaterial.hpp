// 材质基类
#pragma once
#include "IRenderStage.hpp"
#include "RenderPass.hpp"
#include "GraphicPass.hpp"

#include "ShaderResourceManager.hpp"
#include "SamplerManager.h"
#include "ImageManager.h"

#include "DescriptorSetWrapper.hpp"

namespace LT {
	class Pipeline;

	struct MaterialSlot {
		int64_t nSrcID;
		union {
			vk::DescriptorBufferInfo bufferInfo;
			vk::DescriptorImageInfo imageInfo;
		};

		vk::DescriptorType eDescType;
		// 如果是纹理 nSrcID是一个ImageID
		// 如果是const buffer, nSrcID是一个ConstBufferHandle，由ShaderResource控制


		MaterialSlot(vk::DescriptorType desc, int64_t srcID)
			: eDescType(desc)
			, nSrcID(srcID)
		{
		}

		MaterialSlot()
			: eDescType(vk::DescriptorType::eUniformBuffer)
			, nSrcID(-1)
		{
		}
	};

	struct MaterialBindInfo {
		RenderStageType eStage;
		RenderPassFlag nFlag;
		FlightFrameIndex nFlightIndex;
		CommandBufferWrapper* pCmdBuffer;
		Pipeline* pPipeline;
	};

	using MaterialPropDataLayout = std::map<MtlProp, size_t>;

	// 储存材质属性的Desc Set的结构体
	struct MtlPropDescriptorSets {
		std::array<DescriptorSetWrapper, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_descriptorSets;

		MtlPropDescriptorSets();

		void Init(const vk::DescriptorSetLayout& layout);

		void Release();

		DescriptorSetWrapper& GetDescriptorSet(FlightFrameIndex nFlightIndex);
		operator bool() const;
	};


	class IMaterial {
		friend class MaterialManager;
	protected:
		MaterialID m_nID;
		MaterialType m_eMtlType;
		// eMtlPropTex0 - eMtlPropTex15
		// eLighting
		// 共17位是有效的
		RenderPassFlag m_nMtlFlag;
		std::unordered_map<BindingInfo, MaterialSlot, BindingInfoHash> m_mapSlots;

	protected:
		IMaterial(MaterialID nID, MaterialType eType);

		virtual void RegisterStage(RenderStageType eStage) = 0;
	public:
		using _RenderPassMap = std::map<RenderPassFlag, RenderPass*>;
		using RenderPassMap = std::map<RenderStageType, _RenderPassMap>;

		virtual ~IMaterial();

		virtual RenderPass* Bind(const MaterialBindInfo& sMtlBindInfo) = 0;
		//virtual void UpdateMtlResource(RenderStageType eStage, RenderPassFlag nFlag) = 0;

		virtual void BindShaderResource(const MaterialBindInfo& sMtlBindInfo) = 0;

		virtual EngineResult SetMtlProp(MtlProp eProp, const MtlPropVar& value) = 0;

		EngineResult SetTransBuffer(ConstBufferHandle nHandle);
		EngineResult SetLightBuffer(ConstBufferHandle nHandle);
		MaterialType GetMaterialType() const { return m_eMtlType; }

		EngineResult SetSlotSrc(const BindingInfo& sBindingInfo, int64_t nSrcID);

		MaterialID GetMaterialID()const { return m_nID; }

		RenderPassFlag GetMaterialFlag() const { return m_nMtlFlag; }

	};

	//  ┌───────────────────────────────────┐
	//  │      Register Material Prop       │
	//  │     Called By MaterialManager     │
	//  │        Init Mtl Prop Desc         │
	//  └─────────────────┬─────────────────┘
	//                    │
	//                    │
	//┌───────────────────┴────────────────────┐
	//│  Init Material Descriptor Set Layout   │
	//│       Called By Material Manager       │
	//│    vk::DescritporSetLayout Created     │
	//└───────────────────┬────────────────────┘
	//                    │
	//                    │
	//                    │
	//    ┌───────────────┴────────────────┐
	//    │  Init Material Descritpor Set  │
	//    │   vk::DescritporSet Created    │
	//    └───────────────┬────────────────┘
	//                    │
	//                    │
	//┌───────────────────┴─────────────────────┐
	//│When Drawing:                            │
	//│Init RenderPass.PipelineLayout Created.  │
	//└─────────────────────────────────────────┘
} // namespace LT
