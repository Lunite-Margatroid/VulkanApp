#include "vkRendererCommon.h"
#include "vkContext.h"

#include "SemaphoreManager.hpp"

namespace LT {

	SemaphoreManager* SemaphoreManager::s_pInstance = nullptr;

	SemaphoreManager::SemaphoreWithFence SemaphoreManager::CreateSemaphoreWithFence()
	{
		VK_CONTEXT_DEVICE;
		SemaphoreWithFence sem;
		// 如果空闲semaphore是空的
		if (m_listSemaphoreIdel.empty())
		{
			// 遍历一遍等待list
			// 回收可以回收的semaphore
			for (auto iter = m_listSemaphoreSignal.begin(); iter != m_listSemaphoreSignal.end();)
			{
				if (device.getFenceStatus(iter->m_refFence.GetNativeFence()) == vk::Result::eSuccess) {

					m_listSemaphoreIdel.push_back(iter->m_vkSemaphore);
					iter = m_listSemaphoreSignal.erase(iter);
				}
				else
				{
					++iter;
				}
				
			}
		}

		// 回收之后仍然没有可用的semaphore
		if (m_listSemaphoreIdel.empty())
		{
			sem.m_vkSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo({}));
		}
		else
		{
			sem.m_vkSemaphore = m_listSemaphoreIdel.back();
			m_listSemaphoreIdel.pop_back();
		}
		return sem;
	}

	void SemaphoreManager::Init() {
		if (!s_pInstance)
		{
			s_pInstance = new SemaphoreManager();
		}
	}

	void SemaphoreManager::Release() {
		if (s_pInstance)
		{
			delete s_pInstance;
			s_pInstance = nullptr;
		}
	}

	SemaphoreManager& SemaphoreManager::GetInstance() {
		return *s_pInstance;
	}

	SemaphoreBarrier SemaphoreManager::TimelineBegin(int64_t nTimelineKey)
	{
		auto& mgr = GetInstance();

		TimelineStatus* pTimeline = nullptr;
		auto iter = mgr.m_mapTimelines.find(nTimelineKey);
		if (iter == mgr.m_mapTimelines.end())
		{
			mgr.m_mapTimelines[nTimelineKey] = TimelineStatus();
			pTimeline = &mgr.m_mapTimelines[nTimelineKey];
		}
		else
		{
			pTimeline = &iter->second;

			// timeline重启
			// 确保CPU已经同步
			// 此时m_vecSemphoreWaited已经空闲
			for (vk::Semaphore sem : (pTimeline->m_vecSemphoreWaited))
			{
				mgr.m_listSemaphoreIdel.push_back(sem);
			}
			pTimeline->m_vecSemphoreWaited.clear();
		}

		SemaphoreBarrier barrier;
		// 创建下一个submit需要signal的semaphore
		auto sem = mgr.CreateSemaphoreWithFence();
		barrier.m_vkSemaphoreToSignal = sem.m_vkSemaphore;
		pTimeline->m_vecSemphoreSignal.push_back(sem.m_vkSemaphore);

		return barrier;
	}

	SemaphoreBarrier SemaphoreManager::Barrier(int64_t nTimelineKey, FenceRef refFence, int32_t nBranchFlag0, int32_t nBranchFlag1)
	{
		auto& mgr = GetInstance();

		uint64_t nBranchKey = nBranchFlag1;
		nBranchKey = (nBranchKey << 32) | nBranchFlag0;

		TimelineStatus* pTimeline = nullptr;
		auto iter = mgr.m_mapTimelines.find(nTimelineKey);
		if (iter == mgr.m_mapTimelines.end())
		{
			mgr.m_mapTimelines[nTimelineKey] = TimelineStatus();
			pTimeline = &mgr.m_mapTimelines[nTimelineKey];
		}
		else
		{
			pTimeline = &iter->second;
		}

		SemaphoreBarrier barrier;



		// 该barrier等待的是上一次submit的signal
		barrier.m_vecSempahoresToWait.assign(pTimeline->m_vecSemphoreSignal.begin(), pTimeline->m_vecSemphoreSignal.end());
		pTimeline->m_vecSemphoreSignal.clear();

		// 创建下一个submit需要signal的semaphore
		auto sem = mgr.CreateSemaphoreWithFence();
		barrier.m_vkSemaphoreToSignal = sem.m_vkSemaphore;

		// 把等待的semaphore放入待回收的list
		if (refFence)
		{
			for (int i = 0; i < pTimeline->m_vecSemphoreWaited.size(); i++)
			{
				mgr.m_listSemaphoreSignal.emplace_back(pTimeline->m_vecSemphoreWaited[i], refFence);
			}
		}
		else
		{
			// 直接回收有风险
			for (int i = 0; i < pTimeline->m_vecSemphoreWaited.size(); i++)
			{
				mgr.m_listSemaphoreIdel.emplace_back(pTimeline->m_vecSemphoreWaited[i]);
			}
		}

		// timeline更新状态
		pTimeline->m_vecSemphoreWaited.clear();
		pTimeline->m_vecSemphoreWaited.assign(barrier.m_vecSempahoresToWait.begin(), barrier.m_vecSempahoresToWait.end());

		pTimeline->m_vecSemphoreSignal.push_back(barrier.m_vkSemaphoreToSignal);


		return barrier;
	}

	void SemaphoreManager::TimelineEnd(int64_t nTimelineKey, FenceRef refFence)
	{
		auto& mgr = GetInstance();
		auto iter = mgr.m_mapTimelines.find(nTimelineKey);
		if (iter != mgr.m_mapTimelines.end())
		{
			VK_CONTEXT_DEVICE;

			// 销毁已经发射但是无法等待的的semaphore
			auto & status = iter->second;
			//for (vk::Semaphore sem : status)
			//{
			//	device.destroySemaphore(sem);
			//}

			// 最后一次submit禁止signal
			// 总是成功
			//iter->second.m_vecSemphoreSignal; // 没有发射

			// 把等待的semaphore放入待回收的list
			if (refFence)
			{
				for (int i = 0; i < status.m_vecSemphoreWaited.size(); i++)
				{
					mgr.m_listSemaphoreSignal.emplace_back(status.m_vecSemphoreWaited[i], refFence);
				}
			}
			status.m_vecSemphoreWaited.clear();

			// 把没有发射的semaphore放入空闲列表
			for (int i = 0; i < status.m_vecSemphoreSignal.size(); ++i)
			{
				mgr.m_listSemaphoreIdel.emplace_back(status.m_vecSemphoreSignal[i]);
			}
			status.m_vecSemphoreSignal.clear();

		}
	}

	SemaphoreManager::SemaphoreManager() {}

	SemaphoreManager::~SemaphoreManager() {
		VK_CONTEXT_DEVICE;

		for (auto& semWithFence : m_listSemaphoreIdel)
		{
			device.destroySemaphore(semWithFence);
		}
		m_listSemaphoreIdel.clear();

		for (auto& semWithFence : m_listSemaphoreSignal)
		{
			device.destroySemaphore(semWithFence.m_vkSemaphore);
		}
		m_listSemaphoreSignal.clear();

		for (auto& [key, status] : m_mapTimelines) {
			for (auto& sem : status.m_vecSemphoreSignal)
			{
				device.destroySemaphore(sem);
			}

			for (auto& sem : status.m_vecSemphoreWaited)
			{
				device.destroySemaphore(sem);
			}
			status.m_vecSemphoreSignal.clear();
			status.m_vecSemphoreWaited.clear();
		}
		m_mapTimelines.clear();
	}

} // namespace LT
