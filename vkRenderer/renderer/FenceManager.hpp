#pragma once
#include "ManagerTemplate.hpp"

namespace LT
{
	constexpr size_t FENCE_COUNT_PER_BLOCK = 16;

	class FenceManager {
		// ------------------------ static ----------------------------
	public:
		static void Init();
		static void Release();

		static FenceManager& GetInstance();
		// 相当于占位符 尽量不要用这个函数
		static vk::Fence* GetNativeFencePtr(FenceHandle hFence);
		static vk::Fence GetNativeFence(FenceHandle hFence);
		static uint32_t RefDecrease(FenceHandle hFence);
		static uint32_t RefIncrease(FenceHandle hFence);
	private:

		static FenceManager* s_pInstance;

		struct SGetNativeFencePtr {
			vk::Fence* operator() (FenceHandle hFence) const {
				if (hFence == INVALID_FENCE_HANDLE)
				{
					return nullptr;
				}
				else
				{
					return GetInstance().GetNativeFencePtr(hFence);
				}
			}
		};

		struct SFenceRefIncrease {
			uint32_t operator() (FenceHandle hFence) const {
				if (hFence == INVALID_FENCE_HANDLE)
				{
					return 0;
				}
				else
				{
					return GetInstance().RefIncrease(hFence);
				}
			}
		};
		struct SFenceRefDecrease {
			uint32_t operator() (FenceHandle hFence) const {
				if (hFence == INVALID_FENCE_HANDLE)
				{
					return 0;
				}
				else
				{
					return GetInstance().RefDecrease(hFence);
				}
			}
		};
	public:
		using BaseFenceRef = ResourceRef<FenceHandle, vk::Fence, SGetNativeFencePtr, SFenceRefIncrease, SFenceRefDecrease>;

		class _FenceRef : public BaseFenceRef {

		public:
			_FenceRef() {}

			explicit _FenceRef(FenceHandle id) : BaseFenceRef(id) {
			}

			~_FenceRef() = default;

			vk::Fence GetNativeFence();
		};

	public:
		static _FenceRef CreateFence();
		static vk::Result WaitFence(const _FenceRef& refFence);
	public:
		// 引用计数器
		struct FenceWithRefCount {
			vk::Fence m_vkFence;
			uint32_t m_nRefCount;
			FenceWithRefCount() 
				: m_vkFence(VK_NULL_HANDLE)
				, m_nRefCount(0)
			{}

			void Init();

			uint32_t RefDecrease() {
				if (m_vkFence)
				{
					if (m_nRefCount > 0) m_nRefCount--;
					return m_nRefCount;
				}
				return 0;
			}
			uint32_t RefIncrease() {
				if(m_vkFence)
					return ++m_nRefCount;
				return 0;
			}

			uint32_t GetRefCount() const { return m_nRefCount; }

			void ReleaseForce();
		};
		// FencePool
		struct FencePool {
		private:
			std::vector<std::array<FenceWithRefCount, FENCE_COUNT_PER_BLOCK>> m_vkFences;
			std::stack<size_t> m_stkIdleFence;

		public:
			FenceWithRefCount& GetFenceWithRefCount(size_t index);

			uint32_t RefDecrease(size_t index);
			uint32_t RefIncrease(size_t index);
			_FenceRef AllocateFence();

			~FencePool();
		};

	private:
		FencePool m_sFencePool;

		FenceManager();
		~FenceManager();

		FenceManager(const FenceManager&) = delete;
		FenceManager(FenceManager&&) = delete;

		FenceManager& operator = (const FenceManager&) = delete;
		FenceManager& operator = (FenceManager&&) = delete;

	};

	using FenceRef = FenceManager::_FenceRef;


}// namespace LT