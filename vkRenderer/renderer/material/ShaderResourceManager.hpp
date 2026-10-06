// 材质Shader资源管理器
#pragma once
#include "TypeDef.hpp"
#include "BufferManager.h"

#include "ShaderModuleInfo.hpp"

namespace LT {
	class ConstBuffer;

	constexpr uint32_t BLOCK_COUNT_PER_BUFFER_HEAP = 128;


	// Trans Buffer的binding index
	constexpr uint32_t MTL_TRANS_BUFFER_BINDING_INDEX = 0u;
	constexpr BindingSpace MTL_TRANS_BUFFER_BINDING_SPACE = BindingSpace::eVertexShader;
	// Mtl Prop Buffer的binding index
	constexpr uint32_t MTL_PROP_BINDING_INDEX = 2u;
	constexpr BindingSpace MTL_PROP_BINDING_SPACE = BindingSpace::eVertAndFragShader;
	// Mtl Prop Tex的起始binding index
	constexpr uint32_t MTL_TEX_BINDING_MIN = 3u;
	constexpr BindingSpace MTL_TEX_BINDING_SPACE = BindingSpace::eFragmentShader;

	// 材质Shader资源管理器 单例
	// 管理材质的Shader资源（const buffer等）
	class ShaderResourceManager {
		struct BufferWithCounter {
			std::bitset<BLOCK_COUNT_PER_BUFFER_HEAP> flagRef;
			BufferID nBufferID;
			bool bDirt;
			BufferWithCounter()
				: nBufferID(INVALID_BUFFER_ID)
				, bDirt(false)
			{}

			BufferWithCounter(BufferID bufferID, bool dirt)
				: nBufferID(bufferID)
				, bDirt(dirt)
			{
			}
		};

		struct ConstBufferPool {
			std::vector<BufferWithCounter> m_buffers;
			const size_t m_nSizePerBlock;

			ConstBufferPool(size_t nSizePerBlock)
				: m_nSizePerBlock(nSizePerBlock)
			{}

			~ConstBufferPool()
			{
				for (BufferWithCounter& buffer : m_buffers) {
					if (buffer.flagRef.any())
					{
						LOG_ERROR("The Buffer is being used.");
					}
					BufferManager::DeleteBuffer(buffer.nBufferID);
				}
			}

			BufferWithCounter& GetBuffer(BufferID nID)
			{
				for (BufferWithCounter& buffer : m_buffers)
				{
					if (buffer.nBufferID == nID)
					{
						return buffer;
					}
				}
				RENDERER_ASSERT(false, "The Buffer is not belong to this Pool.");
				return m_buffers.front();
			}


			BufferBinding AllocateBuffer() {
				BufferBinding binding;
				binding.nSize = m_nSizePerBlock;
				bool bNeedAllocate = true;
				for (BufferWithCounter& buffer : m_buffers)
				{
					if (!buffer.flagRef.all()) {
						bNeedAllocate = false;
						for (size_t i = 0; i < buffer.flagRef.size(); ++i)
						{
							if (!buffer.flagRef.test(i))
							{
								binding.nBufferID = buffer.nBufferID;
								binding.nOffset = i * binding.nSize;
								buffer.flagRef.set(i);
								buffer.bDirt = true;
							}
						}
					}
				}

				if (bNeedAllocate)
				{
					BufferWithCounter buffer(
						BufferManager::CreateConstBuffer(BLOCK_COUNT_PER_BUFFER_HEAP * m_nSizePerBlock, nullptr)->GetBufferID(),
						true
					);


					binding.nBufferID = buffer.nBufferID;
					binding.nOffset = 0;
					buffer.flagRef.set(0);
					buffer.bDirt = true;

					m_buffers.push_back(std::move(buffer));
				}

				return binding;
			}

			void ReleaseBuffer(const BufferBinding& binding) {

				RENDERER_ASSERT(m_nSizePerBlock == binding.nSize, "unexpected const buffer size.");

				bool bFound = false;
				for (BufferWithCounter& buffer : m_buffers)
				{
					if (buffer.nBufferID == binding.nBufferID) {
						size_t nIndex = binding.nOffset / binding.nSize;
						RENDERER_ASSERT(buffer.flagRef.test(nIndex), "The Buffer is not used.");
						buffer.flagRef.reset(nIndex);
						break;
					}
				}

				if (!bFound)
				{
					LOG_WARNING("Can't find the Buffer.");
				}
			}

			void UpdateConstBuffer(const BufferBinding& binding, const void* pData, size_t nOffset, size_t nSize)
			{
				BufferWithCounter& buffer = GetBuffer(binding.nBufferID);
				buffer.bDirt = true;

				ConstBuffer* pBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(binding.nBufferID));
				if (pBuffer)
				{
					RENDERER_ASSERT(binding.nSize - nOffset >= nSize, "Out of bounds");
					nOffset = binding.nOffset + nOffset;
					pBuffer->UpdateConstBuffer(pData, nOffset, nSize);
				}
			}

			void UpdateConstBuffer(const BufferBinding& binding, const void* pData)
			{
				UpdateConstBuffer(binding, pData, 0, m_nSizePerBlock);
			}

			void UpdateDeviceConstBuffer() {
				for (BufferWithCounter& buffer : m_buffers)
				{
					if (buffer.bDirt)
					{
						ConstBuffer* pBuffer = dynamic_cast<ConstBuffer*>(BufferManager::GetBuffer(buffer.nBufferID));
						if (pBuffer)
						{
							pBuffer->UpdateConstBuffer();
						}
						else
						{
							LOG_ERROR("The Buffer is not Const Buffer.");
						}
						buffer.bDirt = false;
					}
				}
			}
		};

	private:
		int64_t m_nConstBufferHandleCounter;

		// handle到Binding的映射
		std::map<ConstBufferHandle, BufferBinding> m_mapHandle;
		// Buffer的索引
		// BufferID和引用标记
		// 用于材质属性的Buffer Pool
		std::map<MaterialType, ConstBufferPool> m_mapBuffers;
		// 用于MVPTrans的Buffer Pool
		ConstBufferPool m_sMVPTransBufferPool;

		// 目前仅用于Trans Buffer的DescriptorPool
		vk::DescriptorPool m_vkDescriptorPool;
		// 用于材质属性的Descriptor pool
		vk::DescriptorPool m_vkMtlPropDescriptorPool;
		
		// trans buffer desc layout
		vk::DescriptorSetLayout m_vkTransBufferDescriptorSetLayout;
		// trans buffer Desc set
		std::array<vk::DescriptorSet, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_sTransBufferDescriptor;

		ShaderResourceManager();

	public:
		~ShaderResourceManager();

		// 五法则 禁止拷贝和移动
		ShaderResourceManager& operator = (const ShaderResourceManager&) = delete;
		ShaderResourceManager& operator = (ShaderResourceManager&&) = delete;

		ShaderResourceManager(const ShaderResourceManager&) = delete;
		ShaderResourceManager(ShaderResourceManager&&) = delete;

	private:
		ConstBufferHandle GenConstBufferHandle() {
			return m_nConstBufferHandleCounter++;
		}

		// -------------- static ------------------
	private:
		static ShaderResourceManager* s_pInstance;
	public:
		static void Init();
		static void Release();
		static ShaderResourceManager& GetInstance();

		template<typename TMaterial>
		static void RegisterMaterial() {
			auto& mgr = GetInstance();
			m_mapBuffers[TMaterial::GetMaterialType()] = std::vector<BufferWithCounter>();
		}

		static BufferBinding GetConstBufferBinding(ConstBufferHandle nHandle);

		// 传入的是材质属性所用的Decriptor的统计
		static void CreateDescriptorPool(const std::map<vk::DescriptorType, uint32_t>& mapDescriptorCount);

		static ConstBufferHandle CreateTransBufferHandle();
		static void ReleaseTransBufferHandle(ConstBufferHandle nHandle);
		static void UpdateTransBuffer(ConstBufferHandle nHandle, const void* pData);
		static void UpdateDeviceTransBuffer();

		static void GetTransDescriptorSet();

		static vk::DescriptorSetLayout GetTransBufferDescLayout() {
			
		}

		// 获取Trnas Buffer Layout
		static vk::DescriptorSetLayout GetTransBufferDescriptorSetLayout() {

			ShaderResourceManager& mgr = GetInstance();

			if (!mgr.m_vkTransBufferDescriptorSetLayout)
			{
				vk::Device& device = vkContext::GetVkDevice();

				std::array<vk::DescriptorSetLayoutBinding, 1> bindings;
				bindings[0]
					.setBinding(MTL_TRANS_BUFFER_BINDING_INDEX)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setStageFlags(GetShaderStageFlag(MTL_TEX_BINDING_SPACE))
					;

				vk::DescriptorSetLayoutCreateInfo dslci = {};
				dslci
					.setBindings(bindings)
					;

				mgr.m_vkTransBufferDescriptorSetLayout = device.createDescriptorSetLayout(dslci);

			}

			return mgr.m_vkTransBufferDescriptorSetLayout;
		}


		// 获取Trans Buffer描述符
		static vk::DescriptorSet GetTransBufferDescriptorSet(FlightFrameIndex nFlightIndex) {
			ShaderResourceManager& mgr = GetInstance();

			if (!mgr.m_sTransBufferDescriptor[0])
			{
				vk::Device& device = vkContext::GetVkDevice();

				vk::DescriptorSetAllocateInfo dsai = {};
				dsai
					.setDescriptorPool(mgr.m_vkDescriptorPool)
					.setDescriptorSetCount(1)
					.setPSetLayouts(&mgr.m_vkTransBufferDescriptorSetLayout)
					;

				for (int i = 0; i < mgr.m_sTransBufferDescriptor.size(); i++)
				{
					std::vector<vk::DescriptorSet> descSets = device.allocateDescriptorSets(dsai);
					mgr.m_sTransBufferDescriptor[i]= descSets[0];
				}
			}

			return mgr.m_sTransBufferDescriptor[nFlightIndex];
		}



		template<typename TMaterial>
		static ConstBufferHandle CreateMtlPropBufferHandle()
		{
			ConstBufferHandle nHandle = INVALID_CONST_BUFFER_HANDLE;
			BufferBinding binding;

			auto& mgr = GetInstance();
			auto iter = mgr.m_mapBuffers.find(TMaterial::GetMaterialType());
			if (iter != mgr.m_mapBuffers.end())
			{
				nHandle = mgr.GenConstBufferHandle();
				binding = iter->second.AllocateBuffer();
				mgr.m_mapHandle[nHandle] = binding;
			}

			return nHandle;
		}

		template<typename TMaterial>
		static void ReleaseMtlPropBufferHandle(ConstBufferHandle nHandle) {
			auto& mgr = GetInstance();

			if (mgr.m_mapHandle.find(nHandle) != mgr.m_mapHandle.end())
			{

				auto iter = mgr.m_mapBuffers.find(TMaterial::GetMaterialType());
				if (iter != mgr.m_mapBuffers.end())
				{
					BufferBinding binding = mgr.m_mapHandle[nHandle];
					iter->second.ReleaseBuffer(binding);
				}
				else
				{
					LOG_WARNING("Unregistered Material");
				}

				mgr.m_mapHandle.erase(nHandle);
			}
		}

		template<typename TMaterial>
		static void UpdateMtlPropBuffer(ConstBufferHandle nHandle, const void* pData, size_t nOffset, size_t nSize)
		{
			auto& mgr = GetInstance();
			auto iter = mgr.m_mapHandle.find(nHandle);
			if (iter != mgr.m_mapHandle.end())
			{
				BufferBinding binding = iter->second;
				auto iterPool = mgr.m_mapBuffers.find(TMaterial::GetMaterialType());
				if (iterPool != mgr.m_mapBuffers.end())
				{
					iterPool->second.UpdateConstBuffer(binding, pData, nOffset, nSize);
				}
			}
		}

		static void UpdateDeviceMtlPropBuffer();

		static vk::DescriptorPool GetDescriptorPool() {
			return GetInstance().m_vkDescriptorPool;
		}

		static vk::DescriptorPool GetMtlPropDescriptorPool() {
			return GetInstance().m_vkMtlPropDescriptorPool;
		}
	};

} // namespace LT