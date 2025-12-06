#ifndef __SDL_EVENT_H__
#define __SDL_EVENT_H__

#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define enumdef(k) SDLK_##k,

enum SDL_Keys {
  SDLK_NONE = 0,
  _KEYS(enumdef)
  SDLK_KEYNUM, 
};

enum SDL_EventType {
  SDL_KEYDOWN,
  SDL_KEYUP,
  SDL_USEREVENT, // 用户自定义事件
};

/*
生成事件掩码：
SDL_EVENTMASK(SDL_KEYDOWN) → 0b001 (1)
SDL_EVENTMASK(SDL_KEYUP) → 0b010 (2)
SDL_EVENTMASK(SDL_USEREVENT) → 0b100 (4)
在 SDL_PeepEvents 等函数中用掩码筛选事件类型。
*/
#define SDL_EVENTMASK(ev_type) (1u << (ev_type))

enum SDL_EventAction {
  SDL_ADDEVENT, // 添加事件到队列
  SDL_PEEKEVENT, // 查看事件但不移除
  SDL_GETEVENT, // 获取并移除事件
};

typedef struct {
  uint8_t sym; // 按键值(SDL_Keys 枚举，如 SDLK_A、SDLK_RETURN)
} SDL_keysym;

typedef struct {
  uint8_t type; // 事件类型：SDL_KEYDOWN 或 SDL_KEYUP
  SDL_keysym keysym; // 按键符号信息
} SDL_KeyboardEvent;

typedef struct {
  uint8_t type; // 事件类型：SDL_USEREVENT
  int code; // 用户定义的事件代码
  void *data1; // 用户数据指针1
  void *data2; // 用户数据指针2
} SDL_UserEvent;

// 使用联合体，所有成员共享同一块内存
typedef union {
  // type 位于所有事件结构的开头，用于识别事件类型
  uint8_t type;
  // 根据 type 访问对应的成员（key 或 user）
  SDL_KeyboardEvent key;
  SDL_UserEvent user;
} SDL_Event;

int SDL_PushEvent(SDL_Event *ev);
int SDL_PollEvent(SDL_Event *ev);
int SDL_WaitEvent(SDL_Event *ev);
int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask);
uint8_t* SDL_GetKeyState(int *numkeys);

#endif
