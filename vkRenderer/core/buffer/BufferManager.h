#pragma once
#include "Buffer.h"
#include "StagingBuffer.h"
#include "VertexBuffer.h"

namespace LT {
	class BufferManager {
	private:
		std::map<BufferID, Buffer*> m_mapBuffers;

		int64_t m_nBufferIDCount;

	private:
		BufferManager();
		~BufferManager();

		BufferID GenBufferID();
	private:
		static BufferManager* s_pBufferManagerInstance;

	public:

		static void Init();
		static void Release();

		static BufferManager& GetInstance();

		static VertexBuffer* CreateVertexBuffer(size_t nSize, void *pData, uint64_t vertexCount);
		static StagingBuffer* CreateStagingBuffer(size_t nSize, void* pData);

		static void DeleteBuffer(BufferID nID );
		static void DeleteBuffer(Buffer* pBuffer);
	};

} // namespace LT