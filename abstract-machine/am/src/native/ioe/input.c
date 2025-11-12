#include <am.h>
#include <SDL.h>

#define KEYDOWN_MASK 0x8000

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0; // 队列前端索引(front)和队列后端索引(rear)
static SDL_mutex *key_queue_lock = NULL;

// 创建一个从 SDL 键盘扫描码到 AM 按键码的映射表
// SDL 使用自己的扫描码（SDL_SCANCODE_*）,AM 定义了统一的按键码（AM_KEY_*）
// 需要一个映射表来转换两者
/*
static int keymap[256] = {
  [SDL_SCANCODE_ESCAPE] = AM_KEY_ESCAPE,
  [SDL_SCANCODE_F1] = AM_KEY_F1,
  [SDL_SCANCODE_F2] = AM_KEY_F2,
  // ... 省略数十行 ...
  [SDL_SCANCODE_A] = AM_KEY_A,
  [SDL_SCANCODE_B] = AM_KEY_B,
  [SDL_SCANCODE_SPACE] = AM_KEY_SPACE,
  // ... 等等
};
只初始化有按键对应的索引
其他位置自动为 0（表示无映射）
*/
#define XX(k) [SDL_SCANCODE_##k] = AM_KEY_##k,
static int keymap[256] = {
  AM_KEYS(XX)
};

// 在独立线程中监听并处理 SDL 键盘事件
// 无限循环等待事件,将 SDL 键盘事件转换为 AM 格式,存入循环队列供主程序读取,发送键盘中断通知
static int event_thread(void *args) {
  SDL_Event event;
  while (1) {
    // 阻塞式：如果没有事件，线程挂起（不消耗 CPU）
    // 有事件时：线程唤醒，返回事件
    SDL_WaitEvent(&event);
    switch (event.type) {
      case SDL_QUIT: halt(0); // 窗口关闭,调用 halt(0) 终止程序
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        SDL_Keysym k = event.key.keysym; // 获取键盘事件的按键符号信息
        int keydown = event.key.type == SDL_KEYDOWN; // 判断按键是否按下
        int scancode = k.scancode;
        // keymap 数组只包含 AM 支持的按键, 未映射的按键（如多媒体键）值为 0
        if (keymap[scancode] != 0) {
          // 将 SDL 扫描码转换为 AM 按键码,并添加按键状态标志
          int am_code = keymap[scancode] | (keydown ? KEYDOWN_MASK : 0);
          // 加锁保护共享资源（循环队列）,防止多个线程同时访问导致数据不一致
          SDL_LockMutex(key_queue_lock);
          key_queue[key_r] = am_code;
          key_r = (key_r + 1) % KEY_QUEUE_LEN;
          SDL_UnlockMutex(key_queue_lock);
          // 这个函数在其他文件中定义（platform.c）
          // 在函数内部声明是合法的 C 语法,避免在文件顶部添加声明
          void __am_send_kbd_intr();
          // 通知操作系统有键盘事件, native 平台，通过信号实现
          __am_send_kbd_intr();
        }
        break;
      }
    }
  }
}

void __am_input_init() {
  key_queue_lock = SDL_CreateMutex();
  // 线程函数,线程名称,线程参数
  // 线程创建后就分离运行,不需要等待线程结束,线程会一直运行直到程序退出
  SDL_CreateThread(event_thread, "event thread", NULL);
}

void __am_input_config(AM_INPUT_CONFIG_T *cfg) {
  cfg->present = true; // 表示输入设备可用
}

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  int k = AM_KEY_NONE; // 默认值：没有按键事件,如果队列为空，将返回此

  SDL_LockMutex(key_queue_lock);
  // 如果队列不为空，则从队列中取出第一个按键事件
  if (key_f != key_r) {
    k = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  }
  SDL_UnlockMutex(key_queue_lock);
  
  // 解码按键信息,提取按键状态和按键码
  kbd->keydown = (k & KEYDOWN_MASK ? true : false);
  kbd->keycode = k & ~KEYDOWN_MASK;
}
