#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

void switch_fg_pcb(int);
int screen_w, screen_h;

#define NAME(key) \
  [AM_KEY_##key] = #key,

// __attribute__((used)): 防止编译器优化掉未使用的静态变量,即使代码中未直接引用，也保留该数组
static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++) {
    putch(*((const char *)buf + i));
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;
  const char *key = keyname[ev.keycode];
  snprintf(buf, len, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
  //snprintf(buf, len, "%s %s", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
  if (strcmp(key, "F1") == 0) switch_fg_pcb(0);
  else if (strcmp(key, "F2") == 0) switch_fg_pcb(1);
  else if (strcmp(key, "F3") == 0) switch_fg_pcb(2);
  return strlen(buf);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", screen_w, screen_h);
  return strlen(buf);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  // 模拟设备访问缓慢的情况
  // offset是字节偏移，len以字节为单位, 都需要除4改为以像素为单位
  int pixel_offset = offset / 4;
  int pixel_len = len / 4;
  int y = pixel_offset / screen_w;
  int x = pixel_offset % screen_w;
  assert(x + pixel_len <= screen_w);
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, pixel_len, 1, true);
  return pixel_len * 4;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();

  screen_w = io_read(AM_GPU_CONFIG).width;
  screen_h = io_read(AM_GPU_CONFIG).height;
}
