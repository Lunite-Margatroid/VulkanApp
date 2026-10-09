// 材质Shader资源管理器
#pragma once
#include "TypeDef.hpp"
#include "BufferManager.h"

#include "ShaderModuleInfo.hpp"

#include "RenderPass.hpp"

namespace LT {
	class ConstBuffer;

	constexpr uint32_t BLOCK_COUNT_PER_BUFFER_HEAP = 128;


	// Trans Buffer的binding index
	constexpr uint32_t MTL_TRANS_BUFFER_BINDING_INDEX = 0u;
	constexpr ShaderStage MTL_TRANS_BUFFER_BINDING_SPACE = ShaderStage::eVertexShader;
	// Mtl Prop Buffer的binding index
	constexpr uint32_t MTL_PROP_BINDING_INDEX = 2u;
	constexpr ShaderStage MTL_PROP_BINDING_SPACE = ShaderStage::eVertAndFragShader;
	// Mtl Prop Tex的起始binding index
	constexpr uint32_t MTL_TEX_BINDING_MIN = 3u;
	constexpr ShaderStage MTL_TEX_BINDING_SPACE = ShaderStage::eFragmentShader;

	// 材质Shader资源管理器 单例
	// 管理材质的Shader资源（const buffer等）
	class ShaderResourceManager {
		struct BufferWithCounter {
			std::bitset<BLOCK_COUNT_PER_BUFFER_HEAP> flagRef;
			BufferID nBufferID;
			bool bDirt;

			BufferWithCounter();
			BufferWithCounter(BufferID bufferID, bool dirt);
		};

		struct ConstBufferPool {
			std::vector<BufferWithCounter> m_buffers;
			const size_t m_nSizePerBlock;

			ConstBufferPool(size_t nSizePerBlock);
			ConstBufferPool(const ConstBufferPool&) = delete;
			ConstBufferPool& operator = (const ConstBufferPool&) = delete;
			ConstBufferPool(ConstBufferPool&&) = default;
			ConstBufferPool& operator = (ConstBufferPool&&) = default;
			~ConstBufferPool();

			BufferWithCounter& GetBuffer(BufferID nID);
			BufferBinding AllocateBuffer();
			void ReleaseBuffer(const BufferBinding& binding);
			void UpdateConstBuffer(const BufferBinding& binding, const void* pData, size_t nOffset, size_t nSize);
			void UpdateConstBuffer(const BufferBinding& binding, const void* pData);
			void UpdateDeviceConstBuffer();
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
		std::array<vk::DescriptorSet, RENDERER_DEFAULT_FLIGHT_FRAME_NUM> m_arrTransBufferDescriptorSet;

		ShaderResourceManager();

	public:
		~ShaderResourceManager();

		// 五法则 禁止拷贝和移动
		ShaderResourceManager& operator = (const ShaderResourceManager&) = delete;
		ShaderResourceManager& operator = (ShaderResourceManager&&) = delete;

		ShaderResourceManager(const ShaderResourceManager&) = delete;
		ShaderResourceManager(ShaderResourceManager&&) = delete;

	private:
		ConstBufferHandle GenConstBufferHandle();

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
			mgr.m_mapBuffers.insert({ TMaterial::GetMaterialType(), ConstBufferPool(TMaterial::GetPropBufferSize()) });
		}

		static BufferBinding GetConstBufferBinding(ConstBufferHandle nHandle);

		// 传入的是材质属性所用的Decriptor的统计
		static void CreateDescriptorPool(const std::map<vk::DescriptorType, uint32_t>& mapDescriptorCount);

		static ConstBufferHandle CreateTransBufferHandle();
		static void ReleaseTransBufferHandle(ConstBufferHandle nHandle);
		static void UpdateTransBuffer(ConstBufferHandle nHandle, const void* pData);
		static void UpdateDeviceTransBuffer();

		// 获取Trnas Buffer Layout
		static vk::DescriptorSetLayout GetTransBufferDescriptorSetLayout();


		// 获取Trans Buffer描述符
		static vk::DescriptorSet GetTransBufferDescriptorSet(FlightFrameIndex nFlightIndex);



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

		static vk::DescriptorPool GetDescriptorPool();
		static vk::DescriptorPool GetMtlPropDescriptorPool();
	};

} // namespace LT