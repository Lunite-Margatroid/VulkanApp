// IMesh.h
// 渲染器的Mesh的虚基类
#pragma once


namespace LT {

	using MeshID = int64_t;
	using GenVertexBufferFlag = RenderFlagType;
	constexpr MeshID INVALID_MESH_ID = -1;


	class IMesh {
		friend class MeshManager;

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

		struct SColor {
			float r, g, b, a;
		};

		using VertexIndex = uint32_t;

	protected:
		IMesh(MeshID nID);
		virtual ~IMesh();
	protected:
		MeshID m_nID;
		SPosition* m_pPosition;
		SNormal* m_pNormal;
		STangent* m_pTangent;
		SUV* m_pUV0;
		SUV* m_pUV1;
		SUVW* m_pUVW2;
		SUVW* m_pUVW3;
		SUVW* m_pUVW4;
		VertexIndex* m_pIndex;

		VertexIndex* m_pFaceIndex;
		SNormal* m_pFaceNormal;
		STangent* m_pFaceTangent;

		SColor* m_pVertColor;
		float* m_pVertAO;

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



	private:
		void CheckAndAsignVertexCount(uint32_t nCount);
		void CheckAndAsignFaceCount(uint32_t nCount);

	public:
		virtual ~IMesh() = default;

		MeshID GetID() const;

		uint32_t GetVertexCount()const { return m_nVertexCount; }
		uint32_t GetIndexCount()const { return m_nIndexCount; }
		uint32_t GetFaceCount()const { return m_nFaceCount; }
		uint32_t GetVertexIndexCountOfFace()const { return m_nVertexIndexCountOfFace; }

		// float3
		void SetVertexPosition(const SPosition* pData, uint32_t nCount);
		// 移交内存所有权
		void MoveVertexPosition(SPosition* pData, uint32_t nCount);
		const SPosition* GetVertexPosition()const { return m_pPosition; };

		void SetIndexBuffer(const VertexIndex* pData, uint32_t nCount);
		// 移交内存所有权
		void MoveIndexBuffer(VertexIndex* pData, uint32_t nCount);
		const VertexIndex* GetIndexBuffer()const { return m_pIndex; }


		// 一般是 eTriangleList 不保证其他类型的三角图元正确
		void SetPrimitive(vk::PrimitiveTopology ePrimitive);
		vk::PrimitiveTopology GetPrimitive() const { return m_ePrimitive; }

		void SetNormal(const SNormal* pData, uint32_t nCount);
		// 移交内存所有权
		void MoveNormal(SNormal* pData, uint32_t nCount);
		const SNormal* GetNormal() const { return m_pNormal; }


		void SetTangent(const STangent* pData, uint32_t nCount);
		// 移交内存所有权
		void MoveTangent(STangent* pData, uint32_t nCount);
		const STangent* GetTangent() const { return m_pTangent; }


		// face
		// 0xffffffffu 图元重启动
		void SetFaces(const VertexIndex* pData, uint32_t nCount);
		// 设置面索引 移交内存所有权
		void MoveFaces(VertexIndex* pData, uint32_t nCount);
		const VertexIndex* GetFaces() { return m_pFaceIndex; }

		// 一般是 eTriangleList, eTriangleStrip或eTriangleFan 不保证其他类型的三角图元正确
		void SetFacePrimitive(vk::PrimitiveTopology ePrimitive);
		vk::PrimitiveTopology GetFacePrimitive() const { return m_eFacePrimitive; }


		void SetFaceNormal(const SNormal* pData, uint32_t nCount);
		// 设置面法线 移交内存所有权
		void MoveFaceNormal(SNormal* pData, uint32_t nCount);
		const SNormal* GetFaceNormal()const { return m_pFaceNormal; }

		void SetFaceTangent(const STangent* pData, uint32_t nCount);
		void MoveFaceTangent(STangent* pData, uint32_t nCount);
		const STangent* GetFaceTangent() const { return m_pFaceTangent; }


		// 设置UV
		// nUVIndex只能是{0,1}
		void SetUV(const SUV* pData, uint32_t nCount, int32_t nUVIndex);
		void MoveUV(SUV* pData, uint32_t nCount, int32_t nUVIndex);

		// nUVIndex只能是{2,3,4}
		void SetUVW(const SUVW* pData, uint32_t nCount, int32_t nUVIndex);
		void MoveUVW(SUVW* pData, uint32_t nCount, int32_t nUVIndex);


		const SUV* GetUV0() const { return m_pUV0; }
		const SUV* GetUV1() const { return m_pUV1; }
		const SUVW* GetUVW2() const { return m_pUVW2; }
		const SUVW* GetUVW3() const { return m_pUVW3; }
		const SUVW* GetUVW4() const { return m_pUVW4; }

		void SetAO(const float* pData, uint32_t nCount);
		void MoveAO(float* pData, uint32_t);

		const float* GetAO() const { return m_pVertAO; }

		void SetVertColor(const SColor* pData, uint32_t nCount);
		void MoveVertColor(SColor* pData, uint32_t nCount);
		const SColor* GetVertColor() const { return m_pVertColor; }

		void GenVertexBuffer(std::vector<float>& vecOutVertexBuffer, std::vector<uint32_t>& vecOutIndexBuffer, RenderFlagType& nOutFlag, const GenVertexBufferFlag& nInFlag) const;
	};
} // namespace LT