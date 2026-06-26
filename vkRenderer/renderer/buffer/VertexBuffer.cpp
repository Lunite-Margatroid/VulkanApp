#include "vkRendererCommon.h"
#include "vkContext.h"
#include "VertexBuffer.h"
#include "StagingBuffer.h"

#include "BufferManager.h"
#include "DeviceMemoryManager.h"

namespace LT {

	// ----------- 造一个转换格式枚举的Map --------------------
	struct TypeDimension {
		BufferDataType m_eDataType;
		uint32_t m_nDimension;
		bool operator == (const TypeDimension& other) const{
			return m_eDataType == other.m_eDataType && m_nDimension == other.m_nDimension;
		}
	};


	struct HashFormatMap {
		size_t operator ()(const TypeDimension& typeDim) const {
			const uint64_t& key = *reinterpret_cast<const uint64_t*>(&typeDim);
			return std::hash<uint64_t>{}(key);
		}
	};

	std::unordered_map<TypeDimension, vk::Format, HashFormatMap> g_mapTypeDimension2vkFormat = {
		{{BufferDataType::TypeFloat32, 4}, vk::Format::eR32G32B32A32Sfloat},
		{{BufferDataType::TypeFloat32, 3}, vk::Format::eR32G32B32Sfloat},
		{{BufferDataType::TypeFloat32, 2}, vk::Format::eR32G32Sfloat},
		{{BufferDataType::TypeFloat32, 1}, vk::Format::eR32Sfloat},
	};
	// ---------------------------------------------------

	VertexBuffer::VertexBuffer(BufferID id)
		:BufferDst(id),
		m_nVertexCount(0)
	{

	}

	VertexBuffer::VertexBuffer(BufferID id, size_t nSize, void* pData, uint64_t vertexCount)
		:BufferDst(id, nSize, pData),
		m_nVertexCount(vertexCount)
	{
		if (nSize && pData) {
			UpdateDataToGPU();
		}
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Bind(BindTarget nTarget)
	{
		// TODO
	}

	void VertexBuffer::Unbind()
	{
		// TODO
	}

	bool VertexBuffer::AddVertexChannel(const VertexChannelDesc& channelDesc)
	{
		m_vecVertexChannelDesc.push_back(channelDesc);
		return false;
	}

	void VertexBuffer::GetVertexDesc(std::vector<vk::VertexInputBindingDescription>& bindingDesc, std::vector<vk::VertexInputAttributeDescription>& vertexDesc)
	{
		// 从m_vecVertexChannelDesc创建顶点属性描述

		uint32_t nStride = 0u;

		for (int i = 0; i < m_vecVertexChannelDesc.size(); i++) {
			const VertexChannelDesc& vertChannelDesc = m_vecVertexChannelDesc[i];

#ifndef NDEBUG
			RENDERER_ASSERT(g_mapTypeDimension2vkFormat.contains({ vertChannelDesc.m_eDataType, vertChannelDesc.m_nDimension }), "Format Trnas : No Type");
#endif

			vertexDesc.emplace_back(
				i,	// location
				0,	// binding
				g_mapTypeDimension2vkFormat[{vertChannelDesc.m_eDataType, vertChannelDesc.m_nDimension}],	// format
				vertChannelDesc.m_nOffset	// offset
			);

			nStride += vertChannelDesc.m_nDimension * BufferDataType2Size(vertChannelDesc.m_eDataType);
		}

		bindingDesc.push_back(vk::VertexInputBindingDescription(
			0u,			// binding
			nStride,	// stride
			vk::VertexInputRate::eVertex
		));
	}

	void VertexBuffer::UpdateDataToGPU()
	{
		vk::Device& device = vkContext::GetVkDevice();

		if (!m_vkBuffer)
		{
			// 创建Buffer对象
			vk::BufferCreateInfo bci;
			bci.setSize(m_nSize)
				.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eExclusive)
				;
			m_vkBuffer = device.createBuffer(bci);

			// 分配空间
			DeviceMemoryManager::AllocateMemory(this);
		}
		// 填充
		StagingBuffer* stagingBuffer = BufferManager::CreateStagingBuffer(m_nSize, m_pBuffer);
		DeviceCopy(stagingBuffer, m_nSize);
		BufferManager::DeleteBuffer(stagingBuffer);
	}

	uint64_t VertexBuffer::GetVertexCount() const
	{
		return m_nVertexCount;
	}
} //namespace LT
