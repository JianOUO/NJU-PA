#ifndef __SDL_GENERAL_H__
#define __SDL_GENERAL_H__

#include <stdint.h>
#include <stdio.h>

// 这些是 SDL_Init() 的子系统初始化标志，使用位掩码组合，可同时启用多个子系统
// 视频子系统 初始化图形显示,启用窗口、表面、绘图相关功能
#define SDL_INIT_VIDEO       0x01
// 定时器子系统 初始化定时器,用于 SDL_AddTimer() 等定时功能
#define SDL_INIT_TIMER       0x02
// 音频子系统 初始化音频输出,用于播放声音
#define SDL_INIT_AUDIO       0x04
// 禁用信号处理 不安装信号处理器(如 SIGSEGV), 用于调试场景
#define SDL_INIT_NOPARACHUTE 0x08
//手柄/游戏杆子系统 初始化游戏手柄支持
#define SDL_INIT_JOYSTICK    0x10

int SDL_Init(uint32_t flags);
void SDL_Quit();
char *SDL_GetError();
int SDL_SetError(const char* fmt, ...);
int SDL_ShowCursor(int toggle);
void SDL_WM_SetCaption(const char *title, const char *icon);

typedef struct SDL_mutex {} SDL_mutex;
static inline SDL_mutex* SDL_CreateMutex() { return NULL; }
static inline void SDL_DestroyMutex(SDL_mutex* mutex) { }
static inline int SDL_mutexP(SDL_mutex* mutex) { return 0; }
static inline int SDL_mutexV(SDL_mutex* mutex) { return 0; }

#endif
