// MeshStatc.h
// 渲染器 静态Mesh

#pragma once

#include "IMesh.h"

namespace LT {
	// 确保顶点、法线、切线、UV数量一致
	// 确保独立面的数量、面法线和面切线的数量一致
	// 一旦设置顶点数据 不允许修改
	// 如果重复调用相同SetXxxx设置Mesh数据 抛出MeshRepeatSetMeshData异常
	// 如果调用SetXxxx传入的顶点数量不同 抛出MeshVertexCountError异常
	// 如果调用SetXxxx传入的Face数量不同 抛出MeshFaceCountError异常
	class MeshStatic : public IMesh{
	protected:
		DECLEAR_CUSTOMED_EXCEPTION_DERIVE_FROM_RUNTIME_EXCEPT(MeshVertexCountError, "Vector Count is wrong with Vertex Count.");
		DECLEAR_CUSTOMED_EXCEPTION_DERIVE_FROM_RUNTIME_EXCEPT(MeshFaceCountError, "Vector Count is wrong with Face Count.");
		DECLEAR_CUSTOMED_EXCEPTION_DERIVE_FROM_RUNTIME_EXCEPT(MeshRepeatSetMeshData, "Changing Mesh Data is not supported.");

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

		struct SUV {
			float u, v;
		};

		struct SUVW {
			float u, v, w;
		};

		using VertexIndex = uint32_t;

	protected:
		SPosition* m_pPosition;
		SNormal* m_pNormal;
		STangent* m_pTangent;
		SUV* m_pUV0;
		SUV* m_pUV1;
		SUVW* m_pUVW2;
		SUVW* m_pUVW3;
		VertexIndex* m_pIndex;

		VertexIndex* m_pFaceIndex;
		SNormal* m_pFaceNormal;
		STangent* m_pFaceTangent;


		// 顶点的数量
		// len of m_pPosition array
		uint32_t m_nVertexCount;
		// len of index array
		uint32_t m_nIndexCount;


		// Face图元的数量
		uint32_t m_nFaceCount;
		// array m_pFaceIndex的len
		uint32_t m_nVertexIndexCountOfFace;

		// m_pIndex的图元
		vk::PrimitiveTopology m_ePrimitive;
		// face的图元
		vk::PrimitiveTopology m_eFacePrimitive;
	protected:
		MeshStatic();
		~MeshStatic();

	private:
		void CheckAndAsignVertexCount(uint32_t nCount);
		void CheckAndAsignFaceCount(uint32_t nCount);

	protected:

		// float3
		void SetVertexPosition(const SPosition* pData, uint32_t nCount);
		void SetIndexBuffer(const VertexIndex* pData, uint32_t nCount);
		// 一般是 eTriangleList 不保证其他类型的三角图元正确
		void SetPrimitive(vk::PrimitiveTopology ePrimitive);

		void SetNormal(const SNormal* pData, uint32_t nCount);
		void SetTangent(const STangent* pData, uint32_t nCount);

		// face
		// 0xffffffffu 图元重启动
		void SetFaces(const VertexIndex* pData, uint32_t nCount);
		// 一般是 eTriangleList, eTriangleStrip或eTriangleFan 不保证其他类型的三角图元正确
		void SetFacePrimitive(vk::PrimitiveTopology ePrimitive);
		void SetFaceNormal(const SNormal* pData, uint32_t nCount);
		void SetFaceTangent(const STangent* pData, uint32_t nCount);

		// 设置UV
		// nUVIndex只能是{0,1}
		void SetUV(SUV* pData, uint32_t nCount, int32_t nUVIndex);
		// nUVIndex只能是{2,3}
		void SetUVW(SUVW*pData, uint32_t nCount, int32_t nUVIndex);
	};

}