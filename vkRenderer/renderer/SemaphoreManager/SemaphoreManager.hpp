#pragma once

// 该类管理SemaphoreBarrier
// SemaphoreBarrier在两次submit之间获取
// 输入 <时间线标志> <前一个submit是否进行的标志>
// 输出 <下一个submit应该发射的semaphore> <下一个submit应该等待的semaphore>


#include "FenceManager.hpp"

#include "SemaphoreBarrier.hpp"

namespace LT {

	class SemaphoreManager {
	private:
		struct SemaphoreWithFence {
			vk::Semaphore m_vkSemaphore;
			FenceRef m_refFence;

			SemaphoreWithFence(const vk::Semaphore& vkSem, const FenceRef& fence)
				: m_vkSemaphore(vkSem)
				, m_refFence(fence)
			{}

			SemaphoreWithFence()
				: m_vkSemaphore(VK_NULL_HANDLE)
				, m_refFence(INVALID_FENCE_HANDLE)
			{}
		};
		
		struct TimelineStatus {
			// 上一个submit等待的semaphore
			std::vector<vk::Semaphore> m_vecSemphoreWaited;
			// 上一个submit发射的semaphore
			std::vector<vk::Semaphore> m_vecSemphoreSignal;
		};

		std::list<SemaphoreWithFence> m_listSemaphoreSignal;
		std::list<vk::Semaphore> m_listSemaphoreIdel;

		std::map<int64_t, TimelineStatus> m_mapTimelines;

	private:
		SemaphoreManager();
		~SemaphoreManager();

		// 五法则 禁止拷贝和移动
		SemaphoreManager(const SemaphoreManager&) = delete;
		SemaphoreManager(SemaphoreManager&&) = delete;

		SemaphoreManager& operator = (const SemaphoreManager&) = delete;
		SemaphoreManager& operator = (SemaphoreManager&&) = delete;

		SemaphoreWithFence CreateSemaphoreWithFence();

	private:
		static SemaphoreManager* s_pInstance;
	public:
		static void Init();
		static void Release();

		static SemaphoreManager& GetInstance();

		static SemaphoreBarrier TimelineBegin(int64_t nTimelineKey);
		static SemaphoreBarrier Barrier(int64_t nTimelineKey, FenceRef refFence, int32_t nBranchFlag0 = -1, int32_t nBranchFlag1 = -1);
		static void TimelineEnd(int64_t nTimelineKey, FenceRef refFence);
	};

} // namespace LT
