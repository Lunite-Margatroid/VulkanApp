#include "vkRendererCommon.h"
#include "VertexBuffer.h"
#include "vkContext.h"

namespace LT {

	// ----------- 造一个转换格式枚举的Map --------------------
	struct TypeDimension {
		BufferDataType m_eDataType;
		uint32_t m_nDimension;
	};


	struct HashFormatMap {
		size_t operator ()(const TypeDimension& typeDim) {
			const uint64_t& key = *reinterpret_cast<const uint64_t*>(&typeDim);
			std::hash<uint64_t>(key);
		}
	};

	std::unordered_map<TypeDimension, vk::Format, HashFormatMap> g_mapTypeDimension2vkFormat = {
		{{BufferDataType::TypeFloat32, 4}, vk::Format::eR32G32B32A32Sfloat},
		{{BufferDataType::TypeFloat32, 3}, vk::Format::eR32G32B32Sfloat},
		{{BufferDataType::TypeFloat32, 2}, vk::Format::eR32G32Sfloat},
		{{BufferDataType::TypeFloat32, 1}, vk::Format::eR32Sfloat},
	};
	// ---------------------------------------------------

	VertexBuffer::VertexBuffer()
		:Buffer()
	{

	}

	VertexBuffer::VertexBuffer(size_t nSize)
		:Buffer(nSize)
	{
	}

	void VertexBuffer::Bind(BindTarget nTarget)
	{
		// 从m_vecVertexChannelDesc创建顶点属性描述
		std::vector<vk::VertexInputAttributeDescription> vertexDesc;

		for (int i = 0; i < m_vecVertexChannelDesc.size(); i++) {
			const VertexChannelDesc& vertChannelDesc = m_vecVertexChannelDesc[i];

#ifndef NDEBUG
			RENDERER_ASSERT(g_mapTypeDimension2vkFormat.contains({ vertChannelDesc.m_eDataType, vertChannelDesc.m_nDimension }), "Format Trnas : No Type");
#endif

			// 没有stride吗?
			// 数据布局不同还得重整吗？
			vertexDesc.emplace_back(
				i,	// location
				0,	// binding
				g_mapTypeDimension2vkFormat[{vertChannelDesc.m_eDataType, vertChannelDesc.m_nDimension}],	// format
				vertChannelDesc.m_nOffset	// offset
			);
		}

		// 

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

} //namespace LT
