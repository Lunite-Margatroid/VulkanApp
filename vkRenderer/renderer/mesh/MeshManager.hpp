// Mesh管理器
#include "IMesh.hpp"
#include "vkRendererUtil.hpp"


namespace LT {
	class MeshStatic;

	DECLEAR_SINGLETON_MANAGER_BEGIN(MeshManager, IMesh, MeshID, Mesh)
public:
		static MeshStatic* CreateStaticMesh();
	DECLEAR_SINGLETON_MANAGER_END(MeshManager, IMesh, MeshID, Mesh)

} // namespace