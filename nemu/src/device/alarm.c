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
#include <device/alarm.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_HANDLER 8

static alarm_handler_t handler[MAX_HANDLER] = {};
static int idx = 0;

void add_alarm_handle(alarm_handler_t h) {
  assert(idx < MAX_HANDLER);
  handler[idx ++] = h;
}

static void alarm_sig_handler(int signum) {
  int i;
  for (i = 0; i < idx; i ++) {
    handler[i]();
  }
}

/*
init_alarm 函数的作用：
注册信号处理器：处理 SIGVTALRM 信号
启动虚拟定时器：以 60Hz 频率周期性触发
提供定时机制：让设备能够定期更新（如定时器中断、屏幕刷新等）
这是 NEMU 模拟器中实现定时功能的基础，通过 Linux 的虚拟定时器机制模拟硬件定时器。
*/
void init_alarm() {
  // 创建 sigaction 结构体并清零
  struct sigaction s;
  memset(&s, 0, sizeof(s));
  // 设置信号处理函数
  s.sa_handler = alarm_sig_handler;
  // 使用 sigaction 注册 SIGVTALRM（虚拟定时器信号）的处理函数
  int ret = sigaction(SIGVTALRM, &s, NULL);
  // 检查返回值，失败则终止程序
  Assert(ret == 0, "Can not set signal handler");

  // 创建 itimerval 结构体并清零
  struct itimerval it = {};
  // 设置初始延迟(第一次触发的时间)：
  it.it_value.tv_sec = 0;
  // TIMER_HZ = 60(次/秒)，所以 tv_usec = 1000000(1秒=1000000微秒) / 60 ≈ 16666 微秒
  // 即在16.666ms后第一次触发定时器，之后每隔 16.666ms 触发一次定时器
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  // 设置间隔时间：与初始延迟相同，保持定时器持续触发
  it.it_interval = it.it_value;
  // 使用 setitimer(ITIMER_VIRTUAL, ...) 启动虚拟定时器
  // 定时器到期时发送 SIGVTALRM 信号，触发 alarm_sig_handler 处理函数
  // ITIMER_VIRTUAL: 只计算进程在用户态执行的时间。(这里的指的是NEMU在主机的用户态执行的时间)
  // 我们需要的是"模拟器实际工作时间"，不应受系统调用、I/O 等待影响
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  // 检查返回值，失败则终止程序
  Assert(ret == 0, "Can not set timer");
}

/*
初始化阶段：
init_alarm()
  ├─> 注册 SIGVTALRM 信号处理器
  └─> 启动虚拟定时器（60Hz）

运行时：
定时器到期（每 16.67ms）
  └─> 发送 SIGVTALRM 信号
      └─> 调用 alarm_sig_handler()
          └─> 遍历所有注册的处理函数
              └─> 定时器自动重置，继续倒计时
*/