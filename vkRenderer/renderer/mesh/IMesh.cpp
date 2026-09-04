// Mesh的基类
#include "vkRendererCommon.h"
#include "IMesh.hpp"
#include "RenderPass.hpp"

namespace LT {
	IMesh::IMesh(MeshID nID)
		:m_nID(nID)
	{
	}

	MeshID IMesh::GetID() const {
		return m_nID;
	}


	IMesh::IMesh(MeshID nID)
		: m_nID(nID)
		, m_pPosition(nullptr)
		, m_pNormal(nullptr)
		, m_pTangent(nullptr)
		, m_pUV0(nullptr)
		, m_pUV1(nullptr)
		, m_pUVW2(nullptr)
		, m_pUVW3(nullptr)
		, m_pUVW4(nullptr)
		, m_pIndex(nullptr)
		, m_pFaceIndex(nullptr)
		, m_pFaceNormal(nullptr)
		, m_pFaceTangent(nullptr)
		, m_pVertColor(nullptr)
		, m_pVertAO(nullptr)
		, m_nVertexCount(0u)
		, m_nIndexCount(0u)
		, m_nFaceCount(0u)
		, m_nVertexIndexCountOfFace(0u)
		, m_ePrimitive(vk::PrimitiveTopology::eTriangleList)
		, m_eFacePrimitive(vk::PrimitiveTopology::eTriangleList)
	{

	}


	IMesh::~IMesh() {
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
		DELETE_BUFFER(m_pUVW4);
		DELETE_BUFFER(m_pIndex);
		DELETE_BUFFER(m_pFaceIndex);
		DELETE_BUFFER(m_pFaceNormal);
		DELETE_BUFFER(m_pFaceTangent);

	}

	void IMesh::CheckAndAsignVertexCount(uint32_t nCount)
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

	void IMesh::CheckAndAsignFaceCount(uint32_t nCount)
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
	void IMesh::SetVertexPosition(const SPosition* pData, uint32_t nCount) {
		auto* pPosition = new SPosition[nCount];
		memcpy(pPosition, pData, nCount * sizeof(SPosition));
		MoveVertexPosition(pPosition, nCount);
	}

	void IMesh::MoveVertexPosition(SPosition* pData, uint32_t nCount)
	{
		CheckAndAsignVertexCount(nCount);
		if (m_pPosition)
		{
			throw MeshRepeatSetMeshData();
		}

		m_pPosition = pData;
	}

	void IMesh::SetIndexBuffer(const VertexIndex* pData, uint32_t nCount) {

		auto* pIndex = new VertexIndex[nCount];
		memcpy(pIndex, pData, nCount * sizeof(VertexIndex));
		MoveIndexBuffer(pIndex, nCount);
	}
	void IMesh::MoveIndexBuffer(VertexIndex* pData, uint32_t nCount)
	{
		if (m_pIndex)
		{
			throw MeshRepeatSetMeshData();
		}
		m_nIndexCount = nCount;
		m_pIndex = pData;
	}
	void IMesh::SetPrimitive(vk::PrimitiveTopology ePrimitive) {
		m_ePrimitive = ePrimitive;
	}
	void IMesh::SetNormal(const SNormal* pData, uint32_t nCount) {
		auto* pNormal = new SNormal[nCount];
		memcpy(pNormal, pData, sizeof(SNormal) * nCount);
		MoveNormal(pNormal, nCount);
	}
	void IMesh::MoveNormal(SNormal* pData, uint32_t nCount)
	{
		CheckAndAsignVertexCount(nCount);
		if (m_pNormal)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pNormal = pData;
	}
	void IMesh::SetTangent(const STangent* pData, uint32_t nCount) {
		auto* pTangent = new STangent[nCount];
		memcpy(pTangent, pData, sizeof(STangent) * nCount);
		MoveTangent(pTangent, nCount);
	}
	void IMesh::MoveTangent(STangent* pData, uint32_t nCount)
	{
		CheckAndAsignVertexCount(nCount);
		if (m_pTangent)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pTangent = pData;
	}
	void IMesh::SetFaces(const VertexIndex* pData, uint32_t nCount) {
		auto* pFaceIndex = new VertexIndex[nCount];
		memcpy(pFaceIndex, pData, nCount * sizeof(VertexIndex));
		MoveFaces(pFaceIndex, nCount);
	}
	void IMesh::MoveFaces(VertexIndex* pData, uint32_t nCount)
	{
		if (m_pFaceIndex)
		{
			throw MeshRepeatSetMeshData();
		}
		m_nVertexIndexCountOfFace = nCount;
		m_pFaceIndex = pData;
	}
	void IMesh::SetFacePrimitive(vk::PrimitiveTopology ePrimitive) {
		m_eFacePrimitive = ePrimitive;
	}
	void IMesh::SetFaceNormal(const SNormal* pData, uint32_t nCount) {
		auto* pFaceNormal = new SNormal[nCount];
		memcpy(pFaceNormal, pData, sizeof(SNormal) * nCount);
		MoveFaceNormal(pFaceNormal, nCount);

	}
	void IMesh::MoveFaceNormal(SNormal* pData, uint32_t nCount)
	{
		CheckAndAsignFaceCount(nCount);
		if (m_pFaceNormal)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pFaceNormal = pData;
	}
	void IMesh::SetFaceTangent(const STangent* pData, uint32_t nCount)
	{
		auto* pFaceTangent = new STangent[nCount];
		memcpy(pFaceTangent, pData, sizeof(STangent) * nCount);
		MoveFaceTangent(pFaceTangent, nCount);
	}
	void IMesh::MoveFaceTangent(STangent* pData, uint32_t nCount)
	{
		CheckAndAsignFaceCount(nCount);
		if (m_pFaceTangent)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pFaceTangent = pData;
	}
	void IMesh::SetUV(const SUV* pData, uint32_t nCount, int32_t nUVIndex) {
		if (nUVIndex != 0u && nUVIndex != 1u)
		{
			return;
		}
		auto* pTargetData = new SUV[nCount];
		memcpy(pTargetData, pData, sizeof(SUV) * nCount);
		MoveUV(pTargetData, nCount, nUVIndex);
	}
	void IMesh::MoveUV(SUV* pData, uint32_t nCount, int32_t nUVIndex)
	{
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
		pTargetData = pData;
	}
	void IMesh::SetUVW(const SUVW* pData, uint32_t nCount, int32_t nUVIndex) {
		if (nUVIndex != 2u && nUVIndex != 3u && nUVIndex != 4u)
		{
			return;
		}
		auto* pTargetData = new SUVW[nCount];
		memcpy(pTargetData, pData, sizeof(SUVW) * nCount);
		MoveUVW(pTargetData, nCount, nUVIndex);
	}
	void IMesh::MoveUVW(SUVW* pData, uint32_t nCount, int32_t nUVIndex)
	{
		CheckAndAsignVertexCount(nCount);
		if (nUVIndex != 2u && nUVIndex != 3u && nUVIndex != 4u)
		{
			return;
		}
		SUVW*& pTargetData = (nUVIndex == 2u ? m_pUVW2 : (nUVIndex == 3u ? m_pUVW3 : m_pUVW4));
		if (pTargetData)
		{
			throw MeshRepeatSetMeshData();
		}
		pTargetData = pData;
	}
	void IMesh::SetAO(const float* pData, uint32_t nCount)
	{
		auto* pAO = new float[nCount];
		memcpy(pAO, pData, nCount * sizeof(float));
		MoveAO(pAO, nCount);
	}
	void IMesh::MoveAO(float* pData, uint32_t nCount)
	{
		CheckAndAsignVertexCount(nCount);
		if (m_pVertAO)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pVertAO = pData;
	}
	void IMesh::SetVertColor(const SColor* pData, uint32_t nCount)
	{
		auto* pColor = new SColor[nCount];
		memcpy(pColor, pData, nCount * sizeof(SColor));
		MoveVertColor(pColor, nCount);
	}
	void IMesh::MoveVertColor(SColor* pData, uint32_t nCount)
	{
		CheckAndAsignVertexCount(nCount);
		if (m_pVertColor)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pVertColor = pData;
	}
	void IMesh::GenVertexBuffer(std::vector<float>& vecOutVertexBuffer, std::vector<uint32_t>& vecOutIndexBuffer, RenderFlagType& nOutFlag, const GenVertexBufferFlag& nInFlag) const
	{
		vecOutVertexBuffer.clear();
		
	}
} // namespace LT