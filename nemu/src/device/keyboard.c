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
#include <utils.h>

#define KEYDOWN_MASK 0x8000

#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>

// Note that this is not the standard
// NEMU_KEYS(f) 是一个按键列表宏，接受函数 f 作为参数
// 这个宏会展开成一个包含所有按键的列表, 每个按键通过 f() 处理
// 按键包括功能键、字母、数字、符号、方向键等
#define NEMU_KEYS(f) \
  f(ESCAPE) f(F1) f(F2) f(F3) f(F4) f(F5) f(F6) f(F7) f(F8) f(F9) f(F10) f(F11) f(F12) \
f(GRAVE) f(1) f(2) f(3) f(4) f(5) f(6) f(7) f(8) f(9) f(0) f(MINUS) f(EQUALS) f(BACKSPACE) \
f(TAB) f(Q) f(W) f(E) f(R) f(T) f(Y) f(U) f(I) f(O) f(P) f(LEFTBRACKET) f(RIGHTBRACKET) f(BACKSLASH) \
f(CAPSLOCK) f(A) f(S) f(D) f(F) f(G) f(H) f(J) f(K) f(L) f(SEMICOLON) f(APOSTROPHE) f(RETURN) \
f(LSHIFT) f(Z) f(X) f(C) f(V) f(B) f(N) f(M) f(COMMA) f(PERIOD) f(SLASH) f(RSHIFT) \
f(LCTRL) f(APPLICATION) f(LALT) f(SPACE) f(RALT) f(RCTRL) \
f(UP) f(DOWN) f(LEFT) f(RIGHT) f(INSERT) f(DELETE) f(HOME) f(END) f(PAGEUP) f(PAGEDOWN)

// 定义枚举项名称生成宏
// 末尾的逗号用于枚举项分隔
#define NEMU_KEY_NAME(k) NEMU_KEY_ ## k,

// 利用NEMU_KEYS和NEMU_KEY_NAME宏生成一个包含所有按键的枚举
enum {
  NEMU_KEY_NONE = 0,
  MAP(NEMU_KEYS, NEMU_KEY_NAME)
};

// SDL_KEYMAP(k) 是一个宏，参数 k 是按键名
// SDL_SCANCODE_ ## k 连接成 SDL 扫描码常量
// NEMU_KEY_ ## k 连接成 NEMU 按键码常量
// 将 keymap[SDL扫描码] 赋值为对应的 NEMU按键码
#define SDL_KEYMAP(k) keymap[SDL_SCANCODE_ ## k] = NEMU_KEY_ ## k;
static uint32_t keymap[256] = {};

// 初始化一个映射表，将 SDL 键盘扫描码转换为 NEMU 内部按键码。
static void init_keymap() {
  MAP(NEMU_KEYS, SDL_KEYMAP)
}

// 实现一个循环队列（环形缓冲区），用于缓冲键盘按键事件。

#define KEY_QUEUE_LEN 1024
// key_queue[1024]：存储按键码的数组
// {}：初始化为全 0
static int key_queue[KEY_QUEUE_LEN] = {};
// key_f（front）：队头索引，指向下一个要出队的元素
// key_r（rear）：队尾索引，指向下一个要入队的元素
static int key_f = 0, key_r = 0;

static void key_enqueue(uint32_t am_scancode) {
  // 将按键码放入队尾
  key_queue[key_r] = am_scancode;
  // 队尾索引加1，并取模保证循环
  key_r = (key_r + 1) % KEY_QUEUE_LEN;
  // 如果队尾索引等于队头索引，说明队列已满，抛出异常
  Assert(key_r != key_f, "key queue overflow!");
}

static uint32_t key_dequeue() {
  uint32_t key = NEMU_KEY_NONE;
  // 如果队头索引不等于队尾索引，说明队列不为空
  if (key_f != key_r) {
    // 从队头取出按键码
    key = key_queue[key_f];
    // 队头索引加1，并取模保证循环
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  }
  return key;
}

// 在 device.c 中处理 SDL 事件时调用(device_update())
void send_key(uint8_t scancode, bool is_keydown) {
  // NEMU 处于运行状态, 并且按键在映射表中存在
  if (nemu_state.state == NEMU_RUNNING && keymap[scancode] != NEMU_KEY_NONE) {
    // 将 SDL 扫描码转换为 NEMU 按键码，并添加按键状态标志
    uint32_t am_scancode = keymap[scancode] | (is_keydown ? KEYDOWN_MASK : 0);
    // 将按键码放入队列
    key_enqueue(am_scancode);
  }
}
#else // !CONFIG_TARGET_AM
#define NEMU_KEY_NONE 0

static uint32_t key_dequeue() {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  uint32_t am_scancode = ev.keycode | (ev.keydown ? KEYDOWN_MASK : 0);
  return am_scancode;
}
#endif

// 模拟 Intel 8042 键盘控制器，提供 CPU 读取键盘数据的接口。
// i8042 是 Intel 8042 键盘控制器芯片

// 指向键盘数据端口的指针,在init_i8042()中分配
static uint32_t *i8042_data_port_base = NULL;

static void i8042_data_io_handler(uint32_t offset, int len, bool is_write) {
  // 只读操作, 不允许写入
  assert(!is_write);
  // 仅支持偏移 0（数据端口）
  assert(offset == 0);
  // 从队列取出按键码并写入数据端口
  i8042_data_port_base[0] = key_dequeue();
}

void init_i8042() {
  // 分配 4 字节（一个 uint32_t）
  i8042_data_port_base = (uint32_t *)new_space(4);
  // 初始化数据端口内容为 NEMU_KEY_NONE(无按键)
  i8042_data_port_base[0] = NEMU_KEY_NONE;
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("keyboard", CONFIG_I8042_DATA_PORT, i8042_data_port_base, 4, i8042_data_io_handler);
#else
  // 将键盘数据端口映射到内存映射地址
  add_mmio_map("keyboard", CONFIG_I8042_DATA_MMIO, i8042_data_port_base, 4, i8042_data_io_handler);
#endif
  // 初始化键盘映射表
  IFNDEF(CONFIG_TARGET_AM, init_keymap());
}
