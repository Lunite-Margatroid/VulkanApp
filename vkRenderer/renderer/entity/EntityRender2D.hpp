#pragma once
#include "EntityRender.hpp"

namespace LT {
	class EntityRender2D : public EntityRender {
	protected:
		ModelMat2D m_matModel;
		//VPMatrixBuffer2D m_transBuffer;


	public:
		EntityRender2D(EntityID id);
		~EntityRender2D() = default;

		// 接受一个3x3矩阵
		// 列主序 用于左乘
		void SetModuleMat(const float* pMat) override;
	};
} // namespace LT