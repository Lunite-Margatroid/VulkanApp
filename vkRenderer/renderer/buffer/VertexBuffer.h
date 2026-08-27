#pragma once
#include "Buffer.h"
#include "IBindable.h"
#include "BufferDst.h"

namespace LT {
constexpr uint32_t VERTEX_POSITION_BIT_FLAG		= (1 << 0);
constexpr uint32_t VERTEX_UV0_BIT_FLAG			= (1 << 1);
constexpr uint32_t VERTEX_UV1_BIT_FLAG			= (1 << 2);
constexpr uint32_t VERTEX_UV2_BIT_FLAG			= (1 << 3);
constexpr uint32_t VERTEX_UV3_BIT_FLAG			= (1 << 4);
constexpr uint32_t VERTEX_UV4_BIT_FLAG			= (1 << 5);
constexpr uint32_t VERTEX_NORMAL_BIT_FLAG		= (1 << 6);
constexpr uint32_t VERTEX_TANGENT_BIT_FLAG		= (1 << 7);
constexpr uint32_t VERTEX_BITANGENT_BIT_FLAG	= (1 << 8);
constexpr uint32_t VERTEX_COLOR_BIT_FLAG		= (1 << 9);
constexpr uint32_t VERTEX_AO_BIT_FLAG			= (1 << 10);

constexpr uint32_t VERTEX_UV_CHANNEL_COUNT = 5;
constexpr uint32_t VERTEX_CHANNEL_TOTAL_COUNT = 11;
constexpr const char* VERTEX_CHANNEL_NAME[] = {
	"VERT_POSITION",
	"VERT_UV0",
	"VERT_UV1",
	"VERT_UV2",
	"VERT_UV3",
	"VERT_UV4",
	"VERT_NORMAL",
	"VERT_TANGENT",
	"VERT_BITANGENT",
	"VERT_COLOR",
	"VERT_AO"
};

constexpr uint32_t VERTEX_DIMENSION[] = {
	3, // position
	2,2,3,3,3, // UVs
	3, // normal
	3, // tangent
	3, // bitangent
	4, // color
	1  // AO
};

// Position float3	VERT_POSITION
// UV0 float2	VERT_UV0
// UV1 float2	VERT_UV1
// UV2 float3	VERT_UV2
// UV3 float3	VERT_UV3
// UV4 float3	VERT_UV4
// Normal float3	VERT_NORMAL
// Tangent float3	VERT_TANGENT
// Bitangent float3	VERT_BITANGENT
// Color float4	VERT_COLOR
// AO float	VERT_AO

	enum class VertexChannel {
		Position = VERTEX_POSITION_BIT_FLAG,
		UV = VERTEX_UV0_BIT_FLAG,
		Normal = VERTEX_NORMAL_BIT_FLAG,
		Tangent = VERTEX_TANGENT_BIT_FLAG,
		Bitangent = VERTEX_BITANGENT_BIT_FLAG,
		Color = VERTEX_COLOR_BIT_FLAG,
		AO = VERTEX_AO_BIT_FLAG
	};

	using VertexChannelFlag = uint32_t;

	struct VertexChannelDesc {
		VertexChannel m_eChannelType;
		BufferDataType m_eDataType;
		uint32_t m_nDimension;
		uint32_t m_nOffset;
		// 暂时没用到
		uint32_t m_nStride;
		VertexChannelDesc(
			VertexChannel eChannelType,
			BufferDataType eDataType,
			uint32_t nDimension,
			uint32_t nOffset
		)
			:m_eChannelType(eChannelType),
			m_eDataType(eDataType),
			m_nDimension(nDimension),
			m_nOffset(nOffset)
		{
			m_nStride = nDimension * BufferDataType2Size(eDataType);
		}

		VertexChannelDesc(
			VertexChannel eChannelType,
			BufferDataType eDataType,
			uint32_t nDimension,
			uint32_t nOffset,
			uint32_t nStride
		)
			:m_eChannelType(eChannelType),
			m_eDataType(eDataType),
			m_nDimension(nDimension),
			m_nOffset(nOffset),
			m_nStride(nStride)
		{
		}
	};

	class StagingBuffer;

	class VertexBuffer : public BufferDst, public IBindable{
		friend class BufferManager;
	protected:
		std::vector<VertexChannelDesc> m_vecVertexChannelDesc;
		uint64_t m_nVertexCount;

		VertexBuffer(BufferID id);
		VertexBuffer(BufferID id, size_t nSize, void* pData, uint64_t vertexCount);

		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer(const VertexBuffer&) = delete;

		VertexBuffer& operator = (const VertexBuffer&) = delete;
		VertexBuffer& operator = (VertexBuffer&&) = delete;

		~VertexBuffer();
	public:




		/// <summary>
		/// 
		/// </summary>
		/// <param name="nTarget"> 目前没有任何含义 </param>
		void Bind(BindTarget nTarget) override;
		void Unbind() override;

		bool AddVertexChannel(const VertexChannelDesc& channelDesc);

		void GetVertexDesc(std::vector< vk::VertexInputBindingDescription>&bindingDesc ,std::vector<vk::VertexInputAttributeDescription>& vertexDesc);

		void UpdateDataToGPU() override;

		uint64_t GetVertexCount() const;
	};

}// namespace LT