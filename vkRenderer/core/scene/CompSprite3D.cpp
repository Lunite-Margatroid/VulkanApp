#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "CompSprite3D.hpp"
#include "MeshManager.hpp"
#include "BufferManager.h"

#include "EntityRenderMesh.hpp"

#include "BufferManager.h"

namespace LT {
	CompSprite3D::CompSprite3D()
	:m_matWorld(glm::identity<glm::mat4>()), m_refEntityRender(){
	}
	CompSprite3D::~CompSprite3D() {
		if (m_pVertexBuffer) {
			BufferManager::DeleteBuffer(m_pVertexBuffer);
		}

		if (m_pIndexBuffer)
		{
			BufferManager::DeleteBuffer(m_pIndexBuffer);
		}

	}

	void CompSprite3D::Init(MeshRef refMesh, int nFlag) {
		m_refEntityRender = EntityManager::CreateEntityRenderMesh();
		EntityRenderMesh* pEntity = dynamic_cast<EntityRenderMesh*>(m_refEntityRender.GetPtr());

		auto [pVertexBuffer, pIndexBuffer] = BufferManager::CreateVertexIndexBuffer(refMesh, nFlag);

		m_pVertexBuffer = pVertexBuffer;
		m_pIndexBuffer = pIndexBuffer;

		pEntity->SetVertexBuffer(m_pVertexBuffer);
		pEntity->SetIndexBuffer(m_pIndexBuffer);
	}
	EntityRef CompSprite3D::GetRenderEntity()
	{
		return m_refEntityRender;
	}
} // namespace LT