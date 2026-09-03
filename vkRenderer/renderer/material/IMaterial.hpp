// 材质基类
#pragma once
#include "IRenderStage.hpp"
#include "RenderPass.hpp"

namespace LT {
	using MaterialID = int64_t;

	enum class MaterialType {
		eUndifined = -1,
		eMainTexture
	};


	struct MaterialSlot {
		BindingInfo sBindingInfo;
		vk::DescriptorType eDescType;
		int64_t nSrcID;
	};

	class IMaterial {
	protected:
		using RenderPassMap = std::map<RenderPassFlag, RenderPass*>;
	protected:
		MaterialID m_nID;
		std::map<RenderStageType, RenderPassMap> m_mapRenderPasses;
		std::vector<MaterialSlot> m_vecSlots;

	protected:
		IMaterial(MaterialID nID);
		~IMaterial();

		void RegisterStage(RenderStageType eStage);

	public:
		virtual RenderPass* GetRenderPass(RenderStageType eStage, RenderPassFlag nFlag) = 0;
	};
} // namespace LT