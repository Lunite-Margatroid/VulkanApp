#pragma once
namespace LT
{

	class FenceManager {
	public:
		struct FenceWithRefCount {
			vk::Fence vkFence;
			uint32_t nRefCount;
			FenceWithRefCount() 
				: vkFence(VK_NULL_HANDLE)
				, nRefCount(0)
			{}

			void Init();

			uint32_t RefDecrease() {
				if (vkFence)
				{
					if (nRefCount > 0) nRefCount--;
					return nRefCount;
				}
				return 0;
			}
			uint32_t RefIncrease() {
				if(vkFence)
					return ++nRefCount;
				return 0;
			}
		};

		struct FencePool {
			std::vector<FenceWithRefCount> m_vkFences;
			uint32_t RefDecrease(size_t index);
			uint32_t RefIncrease(size_t index);
			size_t AllocateFence();
		};

		std::map<FenceHandle, size_t> m_mapFences;

		FenceManager();

		FenceManager(const FenceManager&) = delete;
		FenceManager(FenceManager&&) = delete;

		FenceManager& operator = (const FenceManager&) = delete;
		FenceManager& operator = (FenceManager&&) = delete;


		// ------------------------ static ----------------------------
	public:
		static void Init();
		static void Release();

		static FenceManager& GetInstance();
		static FenceHandle CreateFence();
		static void AttackHandle(FenceHandle hDst, FenceHandle hSrc);
		static vk::Result WaitFence(FenceHandle);
	};

}// namespace LT