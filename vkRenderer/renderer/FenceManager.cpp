#include "vkRendererCommon.h"
#include "FenceManager.hpp"

namespace LT {

	void FenceManager::FenceWithRefCount::Init() {
		
	}

	uint32_t FenceManager::FencePool::RefDecrease(size_t index)
	{
		if(index >= m_vkFences.size())
			return 0;
		m_vkFences[index].RefDecrease();
	}
	uint32_t FenceManager::FencePool::RefIncrease(size_t index)
	{
		if (index >= m_vkFences.size())
			return 0;
		m_vkFences[index].RefIncrease();
	}
	size_t FenceManager::FencePool::AllocateFence()
	{
		for (size_t i =0; i < m_vkFences.size();i++)
		{
			if (m_vkFences[i].nRefCount == 0)
				return i;
		}
		m_vkFences.push_back();

		return size_t();
	}
} // namspace LT