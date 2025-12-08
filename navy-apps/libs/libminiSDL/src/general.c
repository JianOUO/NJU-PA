#include <NDL.h>
#include <stdio.h>
#include <assert.h>

uint32_t sdl_init_time;

// 初始化指定的 SDL 子系统
int SDL_Init(uint32_t flags) {
  int ret = NDL_Init(flags);
  sdl_init_time = NDL_GetTicks(); 
  return ret;
}

// 清理所有已初始化的 SDL 子系统，释放资源
void SDL_Quit() {
  NDL_Quit();
}

// 获取最后一次 SDL 操作的错误信息字符串
char *SDL_GetError() {
  return "Navy does not support SDL_GetError()";
}

// 设置 SDL 错误信息
int SDL_SetError(const char* fmt, ...) {
  assert(0);
  return -1;
}

/*
控制鼠标光标的显示/隐藏
toggle：
1 或 SDL_ENABLE：显示光标
0 或 SDL_DISABLE：隐藏光标
返回值：
之前的光标状态
*/
int SDL_ShowCursor(int toggle) {
  assert(0);
  return 0;
}

// 设置窗口标题和图标标题
// title：窗口标题字符串 icon：图标标题字符串（通常为 NULL）
void SDL_WM_SetCaption(const char *title, const char *icon) {
}
