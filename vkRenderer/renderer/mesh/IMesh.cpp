// Mesh的基类
#include "vkRendererCommon.h"
#include "IMesh.hpp"

namespace LT {
	IMesh::IMesh(MeshID nID)
		:m_nID(nID)
	{
	}

	MeshID IMesh::GetID() const {
		return m_nID;
	}
} // namespace LT