// Mesh的基类
#include "vkRendererCommon.h"
#include "IMesh.hpp"
#include "RenderPass.hpp"

namespace LT {
	MeshID IMesh::GetID() const {
		return m_nID;
	}


	IMesh::IMesh(MeshID nID)
		: m_nID(nID)
		, m_pPosition(nullptr)
		, m_pNormal(nullptr)
		, m_pTangent(nullptr)
		, m_pBitangent(nullptr)
		, m_pUV0(nullptr)
		, m_pUV1(nullptr)
		, m_pUVW2(nullptr)
		, m_pUVW3(nullptr)
		, m_pUVW4(nullptr)
		, m_pIndex(nullptr)
		, m_pFaceIndex(nullptr)
		, m_pFaceNormal(nullptr)
		, m_pFaceTangent(nullptr)
		, m_pFaceBitangent(nullptr)
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
		DELETE_BUFFER(m_pBitangent);
		DELETE_BUFFER(m_pUV0);
		DELETE_BUFFER(m_pUV1);
		DELETE_BUFFER(m_pUVW2);
		DELETE_BUFFER(m_pUVW3);
		DELETE_BUFFER(m_pUVW4);
		DELETE_BUFFER(m_pIndex);
		DELETE_BUFFER(m_pFaceIndex);
		DELETE_BUFFER(m_pFaceNormal);
		DELETE_BUFFER(m_pFaceTangent);
		DELETE_BUFFER(m_pFaceBitangent);

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

	void IMesh::SetBitangent(const SBitangent* pData, uint32_t nCount) {
		auto* pBitangent = new SBitangent[nCount];
		memcpy(pBitangent, pData, sizeof(SBitangent) * nCount);
		MoveBitangent(pBitangent, nCount);
	}
	void IMesh::MoveBitangent(SBitangent* pData, uint32_t nCount)
	{
		CheckAndAsignVertexCount(nCount);
		if (m_pBitangent)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pBitangent = pData;
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
	void IMesh::SetFaceBitangent(const SBitangent* pData, uint32_t nCount)
	{
		auto* pFaceBitangent = new SBitangent[nCount];
		memcpy(pFaceBitangent, pData, sizeof(SBitangent) * nCount);
		MoveFaceBitangent(pFaceBitangent, nCount);
	}
	void IMesh::MoveFaceBitangent(SBitangent* pData, uint32_t nCount)
	{
		CheckAndAsignFaceCount(nCount);
		if (m_pFaceBitangent)
		{
			throw MeshRepeatSetMeshData();
		}
		m_pFaceBitangent = pData;
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
	void IMesh::GenVertexBuffer(std::vector<float>& vecOutVertexBuffer, std::vector<uint32_t>& vecOutIndexBuffer, RenderFlagType& nOutFlag, const GenVertexBufferFlag& nInFlag, int nInState) const
	{
		vecOutVertexBuffer.clear();
		vecOutIndexBuffer.clear();

		// 各通道bit对应的数据源 数组索引和bit位一致
		// 输出顺序与GraphicPass::GenVertexAttributeDesc保持一致 按bit位从小到大排列
		const float* arrChannelData[VERTEX_CHANNEL_TOTAL_COUNT] = {
			reinterpret_cast<const float*>(m_pPosition),	// VERT_POSITION
			reinterpret_cast<const float*>(m_pUV0),		// VERT_UV0
			reinterpret_cast<const float*>(m_pUV1),		// VERT_UV1
			reinterpret_cast<const float*>(m_pUVW2),		// VERT_UV2
			reinterpret_cast<const float*>(m_pUVW3),		// VERT_UV3
			reinterpret_cast<const float*>(m_pUVW4),		// VERT_UV4
			reinterpret_cast<const float*>(m_pNormal),		// VERT_NORMAL
			reinterpret_cast<const float*>(m_pTangent),		// VERT_TANGENT
			reinterpret_cast<const float*>(m_pBitangent),	// VERT_BITANGENT
			reinterpret_cast<const float*>(m_pVertColor),	// VERT_COLOR
			m_pVertAO,										// VERT_AO
		};

		// 实际写入的通道 决定输出vertex buffer的layout
		std::vector<uint32_t> vecEmitChannelBit;
		uint32_t nStride = 0;
		RenderFlagType nVertexChannelFlag = 0;
		for (uint32_t i = 0; i < VERTEX_CHANNEL_TOTAL_COUNT; i++)
		{
			if ((nInFlag & (1ull << i)) && arrChannelData[i])
			{
				vecEmitChannelBit.push_back(i);
				nVertexChannelFlag |= (1ull << i);
				nStride += VERTEX_DIMENSION[i];
			}
		}

		// 交错排列 每个顶点的各通道按bit位从小到大排列
		vecOutVertexBuffer.reserve(static_cast<size_t>(nStride) * m_nVertexCount);
		for (uint32_t v = 0; v < m_nVertexCount; v++)
		{
			for (uint32_t nBit : vecEmitChannelBit)
			{
				const float* pData = arrChannelData[nBit] + static_cast<size_t>(v) * VERTEX_DIMENSION[nBit];
				vecOutVertexBuffer.insert(vecOutVertexBuffer.end(), pData, pData + VERTEX_DIMENSION[nBit]);
			}
		}

		// 索引buffer
		if (m_pIndex)
		{
			vecOutIndexBuffer.assign(m_pIndex, m_pIndex + m_nIndexCount);
		}

		// 输出flag 保留顶点通道以外的bit 通道bit更新为实际写入的通道
		nOutFlag = (nInFlag & ~(static_cast<RenderFlagType>(VertexChannel::VertexChannelMask))) | nVertexChannelFlag;
	}
	RenderFlagType IMesh::GetRenderPassFlag(int nFlag) const
	{
		// 形参nFlag暂时不使用
		RenderFlagType nFlagOut = 0;

		// 顶点通道 仅包含实际存在数据的通道
		if (m_pPosition)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::Position);
		}
		if (m_pUV0)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::UV);
		}
		if (m_pUV1)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::UV) << 1;
		}
		if (m_pUVW2)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::UV) << 2;
		}
		if (m_pUVW3)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::UV) << 3;
		}
		if (m_pUVW4)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::UV) << 4;
		}
		if (m_pNormal)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::Normal);
		}
		if (m_pTangent)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::Tangent);
		}
		if (m_pBitangent)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::Bitangent);
		}
		if (m_pVertColor)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::Color);
		}
		if (m_pVertAO)
		{
			nFlagOut |= static_cast<RenderFlagType>(VertexChannel::AO);
		}

		// m_pIndex的图元类型
		SetPrimitiveTopology(nFlagOut, m_ePrimitive);

		return nFlagOut;
	}
} // namespace LT