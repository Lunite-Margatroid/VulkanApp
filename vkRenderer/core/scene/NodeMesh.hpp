// Mesh 节点

#pragma once

#include "Node.hpp"
#include "MeshManager.hpp"

namespace LT {
	// Mesh节点 比Node多一个Mesh字段
	class NodeMesh : public Node {
		friend class SceneManager;

	protected:
		MeshRef m_refMesh;

		NodeMesh(NodeID id);
		virtual ~NodeMesh() = default;

	public:


		void SetMesh(const MeshRef& refMesh);
		const MeshRef& GetMesh() const;

		virtual NodeType GetNodeType() const override { return NodeType::eNodeMesh; }
	};
} // namespace LT
