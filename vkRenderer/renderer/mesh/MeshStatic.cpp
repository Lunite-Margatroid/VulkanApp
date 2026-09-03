// MeshStatc.cpp
// 渲染器 静态Mesh

#include "vkRendererCommon.h"
#include "vkContext.h"

#include "MeshStatic.hpp"

namespace LT {
	MeshStatic::MeshStatic(MeshID nID)
		: IMesh(nID)
		, m_pPosition(nullptr)
		, m_pNormal(nullptr)
		, m_pTangent(nullptr)
		, m_pUV0(nullptr)
		, m_pUV1(nullptr)
		, m_pUVW2(nullptr)
		, m_pUVW3(nullptr)
		, m_pIndex(nullptr)
		, m_pFaceIndex(nullptr)
		, m_pFaceNormal(nullptr)
		, m_pFaceTangent(nullptr)
		, m_nVertexCount(0u)
		, m_nIndexCount(0u)
		, m_nFaceCount(0u)
		, m_nVertexIndexCountOfFace(0u)
		, m_ePrimitive(vk::PrimitiveTopology::eTriangleList)
		, m_eFacePrimitive(vk::PrimitiveTopology::eTriangleList)
	{
		
	}


	MeshStatic::~MeshStatic() {
#define DELETE_BUFFER(pBuffer)	\
		do {\
			if(pBuffer) {\
				delete[] (pBuffer);\
				(pBuffer) = nullptr;\
			}\
		} while(false)
		DELETE_BUFFER(m_pPosition);
		DELETE_BUFFER(m_pNormal);
		DELETE_BUFFER(m_pTangent);
		DELETE_BUFFER(m_pUV0);
		DELETE_BUFFER(m_pUV1);
		DELETE_BUFFER(m_pUVW2);
		DELETE_BUFFER(m_pUVW3);
		DELETE_BUFFER(m_pIndex);
		DELETE_BUFFER(m_pFaceIndex);
		DELETE_BUFFER(m_pFaceNormal);
		DELETE_BUFFER(m_pFaceTangent);

	}

	void MeshStatic::CheckAndAsignVertexCount(uint32_t nCount)
	{
		if (m_nVertexCount == 0u)
		{
			m_nVertexCount = nCount;
		}
		else
		{
			if (m_nVertexCount != nCount)
			{
				throw MeshVertexCountError();
			}
		}
	}

	void MeshStatic::CheckAndAsignFaceCount(uint32_t nCount)
	{
		if (m_nFaceCount == 0u)
		{
			m_nFaceCount = nCount;
		}
		else
		{
			if (m_nFaceCount != nCount)
			{
				throw MeshFaceCountError();
			}
		}
	}

	// float3
	void MeshStatic::SetVertexPosition(const SPosition* pData, uint32_t nCount){
		CheckAndAsignVertexCount(nCount);
		if (m_pPosition)
		{
			throw MeshRepeatSetMeshData();
		}

		m_pPosition = new SPosition[nCount];
		memcpy(m_pPosition, pData, nCount * sizeof(SPosition));

	}

	void MeshStatic::SetIndexBuffer(const VertexIndex* pData, uint32_t nCount){
		if (m_pIndex)
		{
			throw MeshRepeatSetMeshData();
		}
		m_nIndexCount = nCount;
		m_pIndex = new VertexIndex[nCount];
		memcpy(m_pIndex, pData, nCount * sizeof(VertexIndex));
	}
	void MeshStatic::SetPrimitive(vk::PrimitiveTopology ePrimitive){
		m_ePrimitive = ePrimitive;
	}
	void MeshStatic::SetNormal(const SNormal* pData, uint32_t nCount){
		CheckAndAsignVertexCount(nCount);
		if (m_pNormal)
		{
			throw MeshRepeatSetMeshData();
		}

		m_pNormal = new SNormal[nCount];
		memcpy(m_pNormal, pData, sizeof(SNormal) * nCount);
	}
	void MeshStatic::SetTangent(const STangent* pData, uint32_t nCount){
		CheckAndAsignVertexCount(nCount);
		if (m_pTangent)
		{
			throw MeshRepeatSetMeshData();
		}

		m_pTangent = new STangent[nCount];
		memcpy(m_pTangent, pData, sizeof(STangent) * nCount);
	}
	void MeshStatic::SetFaces(const VertexIndex* pData, uint32_t nCount){
		m_nVertexIndexCountOfFace = nCount;
		m_pFaceIndex = new VertexIndex[nCount];

		memcpy(m_pFaceIndex, pData, nCount * sizeof(VertexIndex));
	}
	void MeshStatic::SetFacePrimitive(vk::PrimitiveTopology ePrimitive){
		m_eFacePrimitive = ePrimitive;
	}
	void MeshStatic::SetFaceNormal(const SNormal* pData, uint32_t nCount){
		CheckAndAsignFaceCount(nCount);
		if (m_pFaceNormal)
		{
			throw MeshRepeatSetMeshData();
		}

		m_pFaceNormal = new SNormal[nCount];
		memcpy(m_pFaceNormal, pData, sizeof(SNormal) * nCount);

	}
	void MeshStatic::SetFaceTangent(const STangent* pData, uint32_t nCount)
	{
		CheckAndAsignFaceCount(nCount);
		if (m_pFaceTangent)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pFaceTangent = new STangent[nCount];
		memcpy(m_pFaceTangent, pData, sizeof(STangent) * nCount);
	}
	void MeshStatic::SetUV(SUV* pData, uint32_t nCount, int32_t nUVIndex){
		CheckAndAsignVertexCount(nCount);
		if (nUVIndex != 0u && nUVIndex != 1u)
		{
			return;
		}
		SUV*& pTargetData = (nUVIndex == 0u ? m_pUV0 : m_pUV1);
		if (pTargetData)
		{
			throw MeshRepeatSetMeshData();
		}
		pTargetData = new SUV[nCount];
		memcpy(pTargetData, pData, sizeof(SUV) * nCount);
	}
	void MeshStatic::SetUVW(SUVW* pData, uint32_t nCount, int32_t nUVIndex) {
		CheckAndAsignVertexCount(nCount);
		if (nUVIndex != 2u && nUVIndex != 3u)
		{
			return;
		}
		SUVW*& pTargetData = (nUVIndex == 2u ? m_pUVW2 : m_pUVW3);
		if (pTargetData)
		{
			throw MeshRepeatSetMeshData();
		}
		pTargetData = new SUVW[nCount];
		memcpy(pTargetData, pData, sizeof(SUVW) * nCount);
	}
} // namespace LT