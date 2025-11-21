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
#include <utils.h>
#include <device/alarm.h>
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>
#endif

void init_map();
void init_serial();
void init_timer();
void init_vga();
void init_i8042();
void init_audio();
void init_disk();
void init_sdcard();
void init_alarm();

void send_key(uint8_t, bool);
void vga_update_screen();

/*
cpu_exec()在执行每条指令之后就会调用device_update()函数, 
这个函数首先会检查距离上次设备更新是否已经超过一定时间, 
若是, 则会尝试刷新屏幕, 并进一步检查是否有按键按下/释放, 以及是否点击了窗口的X按钮; 
否则则直接返回, 避免检查过于频繁, 因为上述事件发生的频率是很低的.
*/
void device_update() {
  // 使用指令计数器来减少 get_time() 调用频率
  static uint32_t inst_counter = 0;
  // 每执行 100 条指令才检查一次时间
  static const uint32_t CHECK_INTERVAL = 100;
  
  inst_counter++;
  if (inst_counter < CHECK_INTERVAL) {
    return;
  }
  inst_counter = 0;
  
  // 静态变量，记录上次更新时间, 首次调用时为 0
  static uint64_t last = 0;
  // 获取当前时间
  uint64_t now = get_time();
  // 如果距离上次更新不足 16.67ms(定时器触发的时间间隔)，直接返回
  // 否则更新 last = now
  if (now - last < 1000000 / TIMER_HZ) {
    return;
  }
  last = now;

  // 如果配置了 VGA，调用 vga_update_screen() 刷新屏幕, 以约 60Hz 的频率更新
  IFDEF(CONFIG_HAS_VGA, vga_update_screen());

#ifndef CONFIG_TARGET_AM
  SDL_Event event;
  // 轮询事件队列，处理所有待处理事件
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      // 如果窗口关闭，设置模拟器状态为 NEMU_QUIT，退出模拟器
      case SDL_QUIT:
        nemu_state.state = NEMU_QUIT;
        break;
#ifdef CONFIG_HAS_KEYBOARD
      // If a key was pressed
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
      // 提取按键扫描码和按下/释放状态, 并调用 send_key() 函数发送到模拟键盘
        uint8_t k = event.key.keysym.scancode;
        bool is_keydown = (event.key.type == SDL_KEYDOWN);
        send_key(k, is_keydown);
        break;
      }
#endif
      default: break;
    }
  }
#endif
}

// 清空 SDL 事件队列，丢弃所有待处理的 SDL 事件。
void sdl_clear_event_queue() {
#ifndef CONFIG_TARGET_AM
  SDL_Event event;
  // 循环调用 SDL_PollEvent() 直到返回 0（队列为空）
  while (SDL_PollEvent(&event));
#endif
}

void init_device() {
  IFDEF(CONFIG_TARGET_AM, ioe_init());
  // 分配设备I/O内存分配池
  init_map();

  // 按需初始化各个设备
  IFDEF(CONFIG_HAS_SERIAL, init_serial());
  IFDEF(CONFIG_HAS_TIMER, init_timer());
  IFDEF(CONFIG_HAS_VGA, init_vga());
  IFDEF(CONFIG_HAS_KEYBOARD, init_i8042());
  IFDEF(CONFIG_HAS_AUDIO, init_audio());
  IFDEF(CONFIG_HAS_DISK, init_disk());
  IFDEF(CONFIG_HAS_SDCARD, init_sdcard());
  
  // 初始化虚拟定时器, 必须放在最后, 因为其他设备可能注册定时器处理函数
  // 例如：init_timer() 会调用 add_alarm_handle(timer_intr)
  // 定时器启动前需要所有处理函数注册完成
  IFNDEF(CONFIG_TARGET_AM, init_alarm());
}
