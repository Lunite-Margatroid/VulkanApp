// MeshStatc.h
// 渲染器 静态Mesh

#pragma once

#include "IMesh.h"

namespace LT {
	class MeshStatic : public IMesh{
	public:
		struct SPosition {
			float x, y, z;
		};

		struct SNormal {
			float x, y, z;
		};
		
		struct STangent {
			float x, y, z;
		};

		using VertexIndex = uint32_t;

	protected:
	

	public:
		// float3
		void SetVertexPosition(SPosition* pData, uint32_t nCount);
		void SetIndexBuffer(VertexIndex* pData, uint32_t nCount);
		void SetPrimitive(vk::PrimitiveTopology ePrimitive);

		void SetNormal(SNormal* pData, uint32_t nCount);
		void SetTangent(STangent* pData, uint32_t nCount);

		
	};

}