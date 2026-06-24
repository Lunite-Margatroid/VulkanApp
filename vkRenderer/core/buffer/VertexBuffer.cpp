#include "vkRendererCommon.h"
#include "vkContext.h"
#include "VertexBuffer.h"



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

	VertexBuffer::VertexBuffer()
		:Buffer(),
		m_nVertexCount(0)
	{

	}

	VertexBuffer::VertexBuffer(size_t nSize, void* pData, uint64_t vertexCount)
		:Buffer(nSize, pData),
		m_nVertexCount(vertexCount)
	{
		if (nSize && pData) {
			UpdateDataToGPU();
		}
	}

	VertexBuffer::~VertexBuffer()
	{
		ReleaseDeviceMemory();
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
		device.destroyBuffer(m_vkBuffer);
		ReleaseDeviceMemory();


		// 创建Buffer对象
		vk::BufferCreateInfo bci;
		bci.setSize(m_nSize)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive)
			;
		m_vkBuffer = device.createBuffer(bci);

		// 分配空间
		vk::MemoryRequirements vkMemRequire = device.getBufferMemoryRequirements(m_vkBuffer);

		vk::PhysicalDevice& phyDevice = vkContext::GetPhysicalDevice();
		vk::PhysicalDeviceMemoryProperties vkMemProp = phyDevice.getMemoryProperties();

		for (uint32_t i = 0; i < vkMemProp.memoryTypeCount; i++) {
			if (vkMemRequire.memoryTypeBits & (1 << i)) {
				if ((vkMemProp.memoryTypes[i].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
					== (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) {

					vk::MemoryAllocateInfo mai;
					mai.setAllocationSize(vkMemRequire.size)
						.setMemoryTypeIndex(i)
						;
					m_vkMemory = device.allocateMemory(mai);
					break;
				}
			}

		}
		RENDERER_ASSERT(m_vkMemory, "Memory Allocation Failed.");

		device.bindBufferMemory(m_vkBuffer, m_vkMemory, 0);

		// 填充
		void* pData = device.mapMemory(m_vkMemory, 0, m_nSize);
		memcpy(pData, m_pBuffer, m_nSize);
		device.unmapMemory(m_vkMemory);


	}

	void VertexBuffer::ReleaseDeviceMemory() {
		vk::Device& device = vkContext::GetVkDevice();
		device.freeMemory(m_vkMemory);
	}

} //namespace LT
