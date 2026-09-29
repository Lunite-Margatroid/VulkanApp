// 材质Shader资源管理器
#pragma once
#include "TypeDef.hpp"

namespace LT {
	class ConstBuffer;

	constexpr uint32_t BLOCK_COUNT_PER_BUFFER_HEAP = 128;

	// 材质Shader资源管理器 单例
	// 管理材质的Shader资源（const buffer等）
	class ShaderResourceManager {

		using BufferWithCounter = std::pair<BufferID, std::bitset<BLOCK_COUNT_PER_BUFFER_HEAP>>;
	private:
		int64_t m_nConstBufferHandleCounter;

		std::map<ConstBufferHandle, BufferBinding> m_mapHandle;
		std::map<MaterialType, std::vector<BufferWithCounter>> m_mapBuffers;

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

		template<typename TMaterial>
		static ConstBufferHandle CreateConstBufferHandle()
		{
			ConstBufferHandle nHandle = INVALID_CONST_BUFFER_HANDLE;
			BufferBinding binding;
			binding.nSize = TMaterial::GetPropBufferSize();

			auto& mgr = GetInstance();
			auto iter = mgr.m_mapBuffers.find(TMaterial::GetMaterialType());
			if (iter != mgr.m_mapBuffers.end())
			{
				nHandle = GenConstBufferHandle();

				bool bNeedAllocate = true;
				for (auto& [nBufferID, bits] : iter->second)
				{
					if (bits.any()) {
						bNeedAllocate = false;
						for (size_t i = 0; i < bits.size(); ++i)
						{
							if (!bits.test(i))
							{
								binding.nBufferID = nBufferID;
								binding.nOffset = i * binding.nSize;
								bits.set();
							}
						}
					}
				}

				if (bNeedAllocate)
				{
					BufferWithCounter buffer = std::make_pair(
						BufferManager::CreateConstBuffer(BLOCK_COUNT_PER_BUFFER_HEAP * TMaterial::GetPropBufferSize(), nullptr)->GetBufferID(),
						std::bitset<BLOCK_COUNT_PER_BUFFER_HEAP>
					);


					binding.nBufferID = buffer.first;
					binding.nOffset = 0;

					iter->second.push_back(std::move(buffer));
				}

				mgr.m_mapHandle[nHandle] = binding;
			}

			return nHandle;
		}

		template<typename TMaterial>
		static void ReleaseConstBufferHandle(ConstBufferHandle nHandle) {
			auto& mgr = GetInstance();

			if (mgr.m_mapHandle.find(nHandle) != mgr.m_mapHandle.end())
			{

				auto iter = mgr.m_mapBuffers.find(TMaterial::GetMaterialType());
				if (iter != mgr.m_mapBuffers.end())
				{
					BufferBinding binding = mgr.m_mapHandle[nHandle];
					std::vector<BufferWithCounter>& buffers = iter->second;

					RENDERER_ASSERT(TMaterial::GetPropBufferSize() == binding.nSize, "unexpected const buffer size.");

					for (auto& [nBufferID, bits] : buffers)
					{
						if (nBufferID == binding.nBufferID)
						{
							size_t nIndex = binding.nOffset / binding.nSize;
							RENDERER_ASSERT(bits.test(nIndex), "The Buffer is not uesed.");

							bits.reset();
						}
					}

				}

				mgr.m_mapHandle.erase(nHandle);
			}
		}

		static void UpdateConstBuffer(ConstBufferHandle nHandle, void* pData, size_t nOffset, size_t nSize);
	};

} // namespace LT