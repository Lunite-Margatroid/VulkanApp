// 计时器

#include "Timer.hpp"

namespace LT
{
	namespace util
	{
		Timer::Timer()
			: m_tpStart(std::chrono::steady_clock::now())
		{
		}

		double Timer::GetTick() const
		{
			return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_tpStart).count();
		}
	}
}
