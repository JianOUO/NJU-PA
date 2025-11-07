/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
/*
头文件	       提供的函数	                用途	                  平台
<sys/time.h>	gettimeofday()		        获取当前时间，精度到微秒	POSIX（Linux/Unix）
<time.h>	    clock_gettime(), clock()	标准时间函数	           C标准库
*/
#include MUXDEF(CONFIG_TIMER_GETTIMEOFDAY, <sys/time.h>, <time.h>)

// 检查时钟频率常量是否为 1,000,000, CLOCKS_PER_SEC 是 C 标准库定义的宏，表示 clock() 函数返回值的单位
IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(CLOCKS_PER_SEC == 1000000, "CLOCKS_PER_SEC != 1000000"));
// clock_t 通常用于存储时间戳
// 32 位 clock_t (范围: 0 ~ 4,294,967,295)
// 如果 CLOCKS_PER_SEC = 1000000
// 最大时间 = 4294967295 / 1000000 ≈ 4295 秒 ≈ 71 分钟
// 超过 71 分钟就溢出了！
// 64 位 clock_t
// 可以表示数百万年的时间跨度
IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(sizeof(clock_t) == 8, "sizeof(clock_t) != 8"));

static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
#if defined(CONFIG_TARGET_AM)
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
#elif defined(CONFIG_TIMER_GETTIMEOFDAY)
  struct timeval now;
  gettimeofday(&now, NULL);
  uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
#else
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;
#endif
  return us;
}

// 从启动开始到现在的时间，单位us
uint64_t get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

void init_rand() {
  srand(get_time_internal());
}
