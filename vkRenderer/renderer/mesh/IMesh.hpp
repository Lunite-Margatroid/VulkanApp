// IMesh.h
// 渲染器的Mesh的虚基类
#pragma once


namespace LT {

	using MeshID = int64_t;
	constexpr MeshID INVALID_MESH_ID = -1;


	class IMesh {

	protected:
		MeshID m_nID;

	protected:
		IMesh(MeshID nID);

	public:
		virtual ~IMesh() = default;

		MeshID GetID() const;

	};
} // namespace LT