#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "vkContext.h"

#include "EntityRender3D.hpp"

namespace LT {
	EntityRender3D::EntityRender3D(EntityID id)
		: EntityRender(id)
		, m_matModel(1.f)
	{
	}

	void EntityRender3D::SetModuleMat(const float* pMat)
	{
		memcpy(glm::value_ptr(m_matModel), pMat, sizeof(float) * 16);

		m_sModelNormalMat.modelMat = m_matModel;
		glm::mat4 matNormal = glm::transpose(glm::inverse(m_matModel));

		memcpy(&m_sModelNormalMat.normalMatCol, glm::value_ptr(matNormal), sizeof(m_sModelNormalMat.normalMatCol));
	}
} // namespace LT