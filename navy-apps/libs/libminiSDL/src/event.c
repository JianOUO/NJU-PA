#include <NDL.h>
#include <SDL.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

// 将事件推入事件队列 ev - 要推送的事件指针
int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

// 非阻塞地检查并获取一个事件 ev - 用于存储事件的结构体指针
int SDL_PollEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

// 阻塞式等待一个事件 event - 用于存储事件的结构体指针
int SDL_WaitEvent(SDL_Event *event) {
  //assert(0);
  return 1;
}

// ev - 事件数组指针 numevents - 要处理的事件数量 action - 操作类型(SDL_ADDEVENT、SDL_PEEKEVENT、SDL_GETEVENT) mask - 事件类型掩码
int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

// 获取当前所有按键的状态数组 numkeys - 输出参数，返回数组大小
// 返回值：按键状态数组指针（每个元素表示对应按键是否按下）
uint8_t* SDL_GetKeyState(int *numkeys) {
  assert(0);
  return NULL;
}
