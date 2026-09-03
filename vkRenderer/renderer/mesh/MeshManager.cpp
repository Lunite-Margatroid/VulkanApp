// Mesh管理器
#include "vkRendererCommon.h"
#include "MeshManager.hpp"
#include "MeshStatic.hpp"

namespace LT {
	IMPLEMENT_SINGLETON_MANAGER(MeshManager, IMesh, MeshID, Mesh)

} // namespace LT