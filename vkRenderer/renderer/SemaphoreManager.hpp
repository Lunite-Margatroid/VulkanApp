#pragma once

// 该类管理SemaphoreBarrier
// SemaphoreBarrier在两次submit之间获取
// 输入 <时间线标志> <前一个submit是否进行的标志>
// 输出 <下一个submit应该发射的semaphore> <下一个submit应该等待的semaphore>