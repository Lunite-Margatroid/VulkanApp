// MeshStatc.h
// 渲染器 静态Mesh

#pragma once

#include "IMesh.hpp"

namespace LT {

	// 确保顶点、法线、切线、UV数量一致
	// 确保独立面的数量、面法线和面切线的数量一致
	// 一旦设置顶点数据 不允许修改
	// 如果重复调用相同SetXxxx设置Mesh数据 抛出MeshRepeatSetMeshData异常
	// 如果调用SetXxxx传入的顶点数量不同 抛出MeshVertexCountError异常
	// 如果调用SetXxxx传入的Face数量不同 抛出MeshFaceCountError异常
	class MeshStatic : public IMesh {
		friend class MeshManager;


	protected:
		MeshStatic(MeshID nID);
		~MeshStatic();
	};

}