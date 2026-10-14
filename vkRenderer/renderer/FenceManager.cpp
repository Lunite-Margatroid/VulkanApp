#include "vkRendererCommon.h"
#include "FenceManager.hpp"

#include "vkContext.h"

namespace LT {

	// ==================== FenceWithRefCount ===============================
	void FenceManager::FenceWithRefCount::Init() {
		if (m_vkFence)
		{
			m_vkFence = vkContext::GetVkDevice().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		}
	}

	void FenceManager::FenceWithRefCount::ReleaseForce()
	{
		vkContext::GetVkDevice().destroyFence(m_vkFence);
		m_vkFence = VK_NULL_HANDLE;
		m_nRefCount = 0;
	}


	// ==================== FencePool ===============================

	FenceManager::FenceWithRefCount& FenceManager::FencePool::GetFenceWithRefCount(size_t index)
	{
		size_t i = index / FENCE_COUNT_PER_BLOCK;
		size_t j = index % FENCE_COUNT_PER_BLOCK;

		RENDERER_ASSERT(i < m_vkFences.size() && j < m_vkFences[i].size(), "FenceManager::FencePool. Out of bound.");
		return m_vkFences[i][j];
	}

	uint32_t FenceManager::FencePool::RefDecrease(size_t index)
	{
		uint32_t nRefCount = GetFenceWithRefCount(index).RefDecrease();
		if (nRefCount == 0)
		{
			m_stkIdleFence.push(index);
		}
		return nRefCount;
	}
	uint32_t FenceManager::FencePool::RefIncrease(size_t index)
	{
		return GetFenceWithRefCount(index).RefIncrease();
	}
	FenceRef FenceManager::FencePool::AllocateFence()
	{
		if (m_stkIdleFence.empty())
		{
			m_vkFences.push_back(std::array<FenceWithRefCount, FENCE_COUNT_PER_BLOCK>());
			for (size_t i = 0; i < m_vkFences.back().size();++i) {
				FenceWithRefCount & fence = m_vkFences.back()[i];
				fence.Init();
				size_t index = (m_vkFences.size() - 1) * FENCE_COUNT_PER_BLOCK + i;
				m_stkIdleFence.push(index);
			}
		}

		size_t nFence = m_stkIdleFence.top();
		RefIncrease(nFence);
		m_stkIdleFence.pop();
		return FenceRef(static_cast<FenceHandle>(nFence));
	}

	FenceManager::FencePool::~FencePool()
	{
		for (auto& fences : m_vkFences)
		{
			for (FenceWithRefCount& fence : fences) {
				if (fence.GetRefCount() > 0) {
					LOG_WARNING("Fense Force Release. But RefCount is not 0.");
				}
				fence.ReleaseForce();
			}
		}
		m_vkFences.clear();
	}

	// ======================== FenceManager ================================
	FenceManager* FenceManager::s_pInstance = nullptr;

	void FenceManager::Init() {
		if (!s_pInstance)
		{
			s_pInstance = new FenceManager();
		}
	}

	void FenceManager::Release() {
		if (s_pInstance)
		{
			delete s_pInstance;
			s_pInstance = nullptr;
		}
	}

	FenceManager& FenceManager::GetInstance() {
		return *s_pInstance;
	}

	vk::Fence FenceManager::_FenceRef::GetNativeFence()
	{
		return FenceManager::GetNativeFence(m_nID);
	}
	vk::Fence* FenceManager::GetNativeFencePtr(FenceHandle hFence)
	{
		auto &mgr = GetInstance();
		return &(mgr.m_sFencePool.GetFenceWithRefCount(static_cast<size_t>(hFence)).m_vkFence);
	}
	vk::Fence FenceManager::GetNativeFence(FenceHandle hFence)
	{
		if (hFence == INVALID_FENCE_HANDLE)
		{
			return VK_NULL_HANDLE;
		}
		auto& mgr = GetInstance();
		return mgr.m_sFencePool.GetFenceWithRefCount(static_cast<size_t>(hFence)).m_vkFence;
	}
	uint32_t FenceManager::RefDecrease(FenceHandle hFence)
	{
		auto& mgr = GetInstance();
		return mgr.m_sFencePool.RefDecrease(static_cast<size_t>(hFence));
	}
	uint32_t FenceManager::RefIncrease(FenceHandle hFence)
	{
		auto& mgr = GetInstance();
		return mgr.m_sFencePool.RefIncrease(static_cast<size_t>(hFence));
	}
	FenceRef FenceManager::CreateFence()
	{
		auto& mgr = GetInstance();
		return static_cast<FenceRef>(mgr.m_sFencePool.AllocateFence());
	}

	FenceManager::FenceManager() {}
	FenceManager::~FenceManager() {}
} // namspace LT