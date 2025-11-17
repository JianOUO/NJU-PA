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

#include <device/map.h>
#include <device/alarm.h>
#include<utils.h>

// rtc_port_base: 指向 RTC 设备寄存器映射空间的基址
// 在初始化里用 new_space 分配，长度至少 8 字节，对应两个 32 位寄存器
static uint32_t *rtc_port_base = NULL;

// 这是设备回调函数，CPU 读对应地址时被触发；len 通常为 4 字节；写不支持
static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
  // 只允许访问偏移 0 或 4，分别表示低 32 位寄存器和高 32 位寄存器
  assert(offset == 0 || offset == 4);
  // 仅在“读高 32 位寄存器”时执行更新逻辑
  /*
  一致性快照：在读高位寄存器时同时刷新低位与高位，
  这样软件按"先读低(0)，再读高(4)"的惯例能拿到同一时刻的64位时间，
  避免跨越更新导致的撕裂
  */
  if (!is_write && offset == 4) {
    // 调用 get_time() 获取当前时间（单位通常为微秒，64 位）
    uint64_t us = get_time();
    // 将时间拆分为低 32 位和高 32 位，分别写入 rtc_port_base[0]与 rtc_port_base[1]。
    rtc_port_base[0] = (uint32_t)us;
    rtc_port_base[1] = us >> 32;
  }
}

#ifndef CONFIG_TARGET_AM
// 定时器中断回调
static void timer_intr() {
  if (nemu_state.state == NEMU_RUNNING) {
    //对外部声明的函数，向 CPU 提示“有外设中断待处理”（设置中断挂起位），
    //随后在取指或检查点由 CPU 侧响应并进入中断处理流程。
    extern void dev_raise_intr();
    dev_raise_intr();
  }
}
#endif

void init_timer() {
  // 为 RTC 设备分配 8 字节的设备空间（两个 32 位寄存器，低/高位）
  rtc_port_base = (uint32_t *)new_space(8);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("rtc", CONFIG_RTC_PORT, rtc_port_base, 8, rtc_io_handler);
#else
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
#endif
  // 注册定时器回调，使虚拟定时器周期性调用 timer_intr()，从而向CPU提示外设中断(由 dev_raise_intr()完成)
  IFNDEF(CONFIG_TARGET_AM, add_alarm_handle(timer_intr));
}
