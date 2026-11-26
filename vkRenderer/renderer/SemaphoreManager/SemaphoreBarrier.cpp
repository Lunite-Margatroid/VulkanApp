#include "vkRendererCommon.h"
#include "vkContext.h"

#include "SemaphoreBarrier.hpp"

namespace LT {
	SemaphoreBarrier::SemaphoreBarrier()
		: m_vkSemaphoreToSignal(VK_NULL_HANDLE)
	{
	}

	const std::vector<vk::Semaphore>& SemaphoreBarrier::GetWaitSemaphores() const
	{
		return m_vecSempahoresToWait;
	}

	const vk::Semaphore& SemaphoreBarrier::GetSignalSemaphore() const
	{
		return m_vkSemaphoreToSignal;
	}

} // namespace LT