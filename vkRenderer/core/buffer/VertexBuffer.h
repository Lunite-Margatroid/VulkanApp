#pragma once
#include "Buffer.h"
#include "IBindable.h"

namespace LT {
#define VERTEX_POSITION_BIT_FLAG	(1 << 4)
#define VERTEX_UV_BIT_FLAG			(1 << 5)
#define VERTEX_NORMAL_BIT_FLAG		(1 << 6)
#define VERTEX_TANGENT_BIT_FLAG		(1 << 7)
#define VERTEX_BITANGENT_BIT_FLAG	(1 << 8)
#define VERTEX_COLOR_BIT_FLAG		(1 << 9)
#define VERTEX_AO_BIT_FLAG			(1 << 10)

	enum class VertexChannel {
		Position = VERTEX_POSITION_BIT_FLAG,
		UV = VERTEX_UV_BIT_FLAG,
		Normal = VERTEX_NORMAL_BIT_FLAG,
		Tangent = VERTEX_TANGENT_BIT_FLAG,
		Bitangent = VERTEX_BITANGENT_BIT_FLAG,
		Color = VERTEX_COLOR_BIT_FLAG,
		AO = VERTEX_AO_BIT_FLAG
	};

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

	class VertexBuffer : public Buffer, public IBindable{
	protected:
		std::vector<VertexChannelDesc> m_vecVertexChannelDesc;
		uint64_t m_nVertexCount;
		vk::DeviceMemory m_vkMemory;
	public:
		VertexBuffer();
		VertexBuffer(size_t nSize, void* pData, uint64_t vertexCount);

		VertexBuffer(VertexBuffer&&) noexcept = default;
		VertexBuffer(const VertexBuffer&) = delete;

		VertexBuffer& operator = (const VertexBuffer&) = delete;
		VertexBuffer& operator = (VertexBuffer&&) noexcept = default;

		~VertexBuffer();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="nTarget"> 目前没有任何含义 </param>
		void Bind(BindTarget nTarget) override;
		void Unbind() override;

		bool AddVertexChannel(const VertexChannelDesc& channelDesc);

		void GetVertexDesc(std::vector< vk::VertexInputBindingDescription>&bindingDesc ,std::vector<vk::VertexInputAttributeDescription>& vertexDesc);

		void UpdateDataToGPU() override;
		void ReleaseDeviceMemory() override;

		uint64_t GetVertexCount() const;

		/// <summary>
		/// 把pStagingBuffer的Device数据拷贝到当前对象
		/// </summary>
		/// <param name="pStagingBuffer"> 源数据 </param>
		/// <param name="nSize"> 拷贝数据的尺寸 </param>
		/// <param name="nSrcOffset"> 源数据的offset </param>
		/// <param name="nDstOffset"> 目标数据的offset </param>
		void DeviceCopy(StagingBuffer* pStagingBuffer, const uint64_t nSize, const uint64_t nSrcOffset = 0u,const uint64_t nDstOffset = 0u);
	};

}// namespace LT