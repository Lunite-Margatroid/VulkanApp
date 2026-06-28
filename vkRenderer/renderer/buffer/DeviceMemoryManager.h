#pragma once
#include "Buffer.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"
#include "IndexBuffer.h"
#include "ConstBuffer.h"

namespace LT {
	class DeviceMemoryManager {
	private:
		std::map<BufferID, vk::DeviceMemory> m_mapVkMemory;

		DeviceMemoryManager();
		~DeviceMemoryManager();


		void AllocateMemory(Buffer& buffer, vk::MemoryPropertyFlags memoryPorp);
	public:
		/// <summary>
		/// 把目标Buffer的Memory映射出来。只有Staging和ConstBuffer有效
		/// </summary>
		/// <param name="buffer"></param>
		/// <returns></returns>
		void* MapMemory(Buffer& buffer);

		/// <summary>
		/// 关闭映射
		/// </summary>
		/// <param name="buffer"></param>
		void UnmapMemory(Buffer& buffer);

	private:
		static DeviceMemoryManager* s_pDeviceMemoryManagerInstance;
	public:
		static void Init();
		static void Release();

		static DeviceMemoryManager& GetInstance();

		static void AllocateMemory(VertexBuffer* vertexBuffer);
		static void AllocateMemory(StagingBuffer* stagingBuffer);
		static void AllocateMemory(IndexBuffer* pIndexBuffer);
		static void AllocateMemory(ConstBuffer* pConstBuffer);
		static void AsignMemory(StagingBuffer* stagingBuffer, size_t nSize, void* pData);
		static void AsignMemory(ConstBuffer* pConstBuffer, size_t nSize, void* pData);
		static void FreeMemory(Buffer& buffer);
		

	};

} // namespace LT