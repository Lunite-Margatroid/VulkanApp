#pragma once
#include "Buffer.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"

namespace LT {
	class DeviceMemoryManager {
	private:
		std::map<BufferID, vk::DeviceMemory> m_mapVkMemory;

		DeviceMemoryManager();
		~DeviceMemoryManager();


		void AllocateMemory(Buffer& buffer, vk::MemoryPropertyFlags memoryPorp);

	private:
		static DeviceMemoryManager* s_pDeviceMemoryManagerInstance;
	public:
		static void Init();
		static void Release();

		static DeviceMemoryManager& GetInstance();

		static void AllocateMemory(VertexBuffer* vertexBuffer);
		static void AllocateMemory(StagingBuffer* stagingBuffer);
		static void AsignMemory(StagingBuffer* stagingBuffer, size_t nSize, void* pData);
		static void FreeMemory(Buffer& buffer);
		

	};

} // namespace LT