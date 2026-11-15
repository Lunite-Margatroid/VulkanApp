#pragma once
#include "EntityRender.hpp"

namespace LT
{

	class EntityRender3D : public EntityRender {
	protected:
		glm::mat4 m_matModel;
		ModelNormalMat m_sModelNormalMat;

	public:
		EntityRender3D(EntityID);
		~EntityRender3D() = default;

		EntityRender3D(EntityRender3D&&) = delete;
		EntityRender3D(const EntityRender3D&) = delete;
		EntityRender3D& operator = (EntityRender3D&&) = delete;
		EntityRender3D& operator = (const EntityRender3D&) = delete;

		// 接受一个4x4矩阵
		// 列主序 用于左乘
		void SetModuleMat(const float* pMat) override;

		virtual void SetMaterial(const MaterialRef& refMtl) = 0;
		virtual MaterialRef GetMaterial() const = 0;
		virtual MaterialID GetMaterialID() const = 0;
	};


} // namespace LT