#include <sdl-file.h>
#include <assert.h>

// 打开文件并创建文件类型的 SDL_RWops 对象
// filename：文件路径 mode：打开模式
SDL_RWops* SDL_RWFromFile(const char *filename, const char *mode) {
  assert(0);
  return NULL;
}

// 内存缓冲区创建内存类型的 SDL_RWops 对象
// mem：内存缓冲区指针 size：缓冲区大小（字节）
SDL_RWops* SDL_RWFromMem(void *mem, int size) {
  assert(0);
  return NULL;
}
