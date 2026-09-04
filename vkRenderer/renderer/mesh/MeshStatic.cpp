// MeshStatc.cpp
// 渲染器 静态Mesh

#include "vkRendererCommon.h"
#include "vkContext.h"

#include "MeshStatic.hpp"

namespace LT {
	MeshStatic::MeshStatic(MeshID nID)
		:IMesh(nID)
	{
	}
	MeshStatic::~MeshStatic()
	{
	}
} // namespace LT