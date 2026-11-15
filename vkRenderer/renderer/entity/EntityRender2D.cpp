#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "EntityRender2D.hpp"

namespace LT {
	EntityRender2D::EntityRender2D(EntityID id)
		: EntityRender(id)
	{}
	void EntityRender2D::SetModuleMat(const float* pMat) {
		m_matModel.modelMat[0][0] = pMat[0];
		m_matModel.modelMat[0][1] = pMat[1];
		m_matModel.modelMat[1][0] = pMat[3];
		m_matModel.modelMat[1][1] = pMat[4];

		m_matModel.offset[0] = pMat[6];
		m_matModel.offset[1] = pMat[7];
	}
}