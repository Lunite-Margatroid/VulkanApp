#include "vkRendererCommon.h"
#include "vkContext.h"
#include "BufferManager.h"
namespace LT {
	BufferManager* BufferManager::s_pBufferManagerInstance = nullptr;

	void BufferManager::Init() {
		s_pBufferManagerInstance = new BufferManager();
	}

	void BufferManager::Release() {
		if (s_pBufferManagerInstance)
		{
			delete s_pBufferManagerInstance;
			s_pBufferManagerInstance = nullptr;
		}
		else
		{
			LOG_WARNING("BufferManager::Release it did not init.");
		}
	}

	BufferManager& BufferManager::GetInstance() {
		RENDERER_ASSERT(s_pBufferManagerInstance, "Buffer Manager did not init.");
		return *s_pBufferManagerInstance;
	}

	BufferManager::BufferManager()
		:m_nBufferIDCount(0)
	{
	}

	BufferManager::~BufferManager()
	{
		if (m_mapBuffers.size() > 0) {
			LOG_ERROR("%s, Buffers did not release.", __FUNCTION__);
		}
	}

	BufferID BufferManager::GenBufferID()
	{
		return static_cast<BufferID>(m_nBufferIDCount++);
	}

	VertexBuffer* BufferManager::CreateVertexBuffer(size_t nSize, void* pData, uint64_t vertexCount)
	{
		BufferManager& bufferManager = GetInstance();

		VertexBuffer* pVertexBuffer = new VertexBuffer(bufferManager.GenBufferID(), nSize, pData, vertexCount);

		RENDERER_ASSERT(pVertexBuffer, "Vertex Buffer create failed.");

		bufferManager.m_mapBuffers[pVertexBuffer->GetBufferID()] = pVertexBuffer;

		return pVertexBuffer;
	}
	StagingBuffer* BufferManager::CreateStagingBuffer(size_t nSize, void* pData)
	{

		BufferManager& bufferManager = GetInstance();

		StagingBuffer* pStagingBuffer = new StagingBuffer(bufferManager.GenBufferID(), nSize, pData);

		RENDERER_ASSERT(pStagingBuffer, "Staging Buffer create failed.");

		bufferManager.m_mapBuffers[pStagingBuffer->GetBufferID()] = pStagingBuffer;

		return pStagingBuffer;
	}
	IndexBuffer* BufferManager::CreateIndexBuffer(size_t nSize, void* pData, uint64_t indexCount)
	{
		BufferManager& bufferManager = GetInstance();
		IndexBuffer* pIndexBuffer = new IndexBuffer(bufferManager.GenBufferID(), nSize, pData, indexCount);
		RENDERER_ASSERT(pIndexBuffer, "Index Buffer create failed.");
		bufferManager.m_mapBuffers[pIndexBuffer->GetBufferID()] = pIndexBuffer;
		return pIndexBuffer;
	}
	void BufferManager::DeleteBuffer(BufferID nID)
	{
		BufferManager& bufferManager = GetInstance();
		auto iter = bufferManager.m_mapBuffers.find(nID);
		if (iter == bufferManager.m_mapBuffers.end())
		{
			LOG_WARNING("Delete Buffer. Invalid ID");
			return;
		}
		delete iter->second;
		bufferManager.m_mapBuffers.erase(iter);
	}
	void BufferManager::DeleteBuffer(Buffer* pBuffer)
	{
		DeleteBuffer(pBuffer->GetBufferID());
	}
} // namespace LT