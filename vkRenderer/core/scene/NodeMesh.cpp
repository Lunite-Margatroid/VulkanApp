// Mesh节点

#include "vkRendererCommon.h"
#include "EngineCommon.h"
#include "NodeMesh.hpp"
#include "CompSprite3D.hpp"

namespace LT {
	NodeMesh::NodeMesh(NodeID id)
		: Node(id), m_refMesh()
	{
		m_arrComponents[static_cast<size_t>(ComponentType::eSprite3D)] = new CompSprite3D();
	}

	void NodeMesh::SetMesh(const MeshRef& refMesh) {
		m_refMesh = refMesh;
	}

	const MeshRef& NodeMesh::GetMesh() const {
		return m_refMesh;
	}
} // namespace LT
