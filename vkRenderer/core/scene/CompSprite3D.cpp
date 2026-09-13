#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "CompSprite3D.hpp"
#include "MeshManager.hpp"
#include "BufferManager.h"

#include "EntityRenderMesh.hpp"

namespace LT {
	CompSprite3D::CompSprite3D()
	:m_matWorld(glm::identity<glm::mat4>()), m_refEntityRender(){
	}
	CompSprite3D::~CompSprite3D() {
	}

	void CompSprite3D::Init(MeshRef refMesh) {
		m_refEntityRender = EntityManager::CreateEntityRenderMesh();
		EntityRenderMesh* pEntity = dynamic_cast<EntityRenderMesh*>(m_refEntityRender.GetPtr());

		if(pEntity) {
			// 初始化渲染实体
			pEntity->SetMesh(refMesh);
		}

	}
} // namespace LT