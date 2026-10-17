// 计时器
#pragma once
#include <chrono>

namespace LT
{
	namespace util
	{
		// 计时器, 用于统计耗时
		class Timer
		{
		public:
			Timer();

			// 返回自对象创建以来经过的毫秒数
			double GetTick() const;

		private:
			std::chrono::steady_clock::time_point m_tpStart; // 创建时刻
		};
	}
}
