#pragma once

namespace LT {
	class SemaphoreBarrier {
	public:
		std::vector<vk::Semaphore> m_vecSempahoresToWait;
		vk::Semaphore m_vkSemaphoreToSignal;

	public:
		SemaphoreBarrier();


		// semaphore to wait
		const std::vector<vk::Semaphore>& GetWaitSemaphores() const;
		// semaphore to signal
		const vk::Semaphore& GetSignalSemaphore() const;

	};

} // namespace LT