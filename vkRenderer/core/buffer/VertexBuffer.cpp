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
		:Buffer()
	{

	}

	VertexBuffer::VertexBuffer(size_t nSize, void* pData)
		:Buffer(nSize, pData)
	{
	
	}

	void VertexBuffer::Bind(BindTarget nTarget)
	{
		vk::BufferCreateInfo bci;
		bci.setSize(m_nSize)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive)
			;


		vk::Device& device = vkContext::GetVkDevice();
		vk::Buffer vkBuffer = device.createBuffer(bci);
		vk::MemoryRequirements vkMemRequire = device.getBufferMemoryRequirements(vkBuffer);
		
		vk::PhysicalDevice& phyDevice = vkContext::GetPhysicalDevice();
		vk::PhysicalDeviceMemoryProperties vkMemProp = phyDevice.getMemoryProperties();
		vk::DeviceMemory vkMem;

		for (uint32_t i = 0; i < vkMemProp.memoryTypeCount; i++) {
			if (vkMemRequire.memoryTypeBits & (1 << i)) {
				if ((vkMemProp.memoryTypes[i].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
					== (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) {
					
					vk::MemoryAllocateInfo mai;
					mai.setAllocationSize(m_nSize)
						.setMemoryTypeIndex(i)
						;
					vkMem = device.allocateMemory(mai);
					break;
				}
			}
			
		}
		RENDERER_ASSERT(vkMem, "Memory Allocation Failed.");

		device.bindBufferMemory(vkBuffer, vkMem, 0);

		void* pData = device.mapMemory(vkMem,0, m_nSize);
		memcpy(pData, m_pBuffer, m_nSize);
		device.unmapMemory(vkMem);

		vkContext::GetInstance().GetCmdBuffer(static_cast<unsigned int>(nTarget));


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

} //namespace LT
