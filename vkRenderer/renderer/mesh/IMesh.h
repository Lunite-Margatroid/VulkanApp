// IMesh.h
// 渲染器的Mesh的虚基类
#pragma once


namespace LT {

	using MeshID = int64_t;

	class IMesh {
	public:
		virtual ~IMesh() {}
	};
} // namespace LT