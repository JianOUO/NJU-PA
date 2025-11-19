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
#include <device/map.h>

// 默认是400x300
#define SCREEN_W (MUXDEF(CONFIG_VGA_SIZE_800x600, 800, 400))
#define SCREEN_H (MUXDEF(CONFIG_VGA_SIZE_800x600, 600, 300))

// AM 目标：通过 io_read(AM_GPU_CONFIG) 向 AM 的 GPU 配置读取实际宽高
// 非 AM：使用上面的编译期常量 SCREEN_W/SCREEN_H。
static uint32_t screen_width() {
  return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).width, SCREEN_W);
}

static uint32_t screen_height() {
  return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).height, SCREEN_H);
}

// 返回帧缓冲区字节数 = 宽 × 高 × 每像素字节数（这里是 4 字节，即 32-bit RGBA）
static uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint32_t);
}

// 视频内存(帧缓冲区),存储屏幕像素数据的缓冲区
// 每个像素 4 字节（32 位），格式：ARGB8888（Alpha, Red, Green, Blue）
// 布局：按行存储，vmem[y * width + x] 表示坐标 (x, y) 的像素
// 程序写入像素，NEMU 读取并显示
static void *vmem = NULL;
// VGA 控制端口基址, 指向 VGA 控制寄存器
// 程序读取屏幕分辨率，用于初始化图形应用
static uint32_t *vgactl_port_base = NULL;

#ifdef CONFIG_VGA_SHOW_SCREEN
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>

// SDL 渲染器，, 负责将纹理绘制到窗口
static SDL_Renderer *renderer = NULL;
// 存储像素数据的纹理对象，作为渲染的源数据
static SDL_Texture *texture = NULL;

static void init_screen() {
  SDL_Window *window = NULL;
  // 用客体 ISA（如 riscv32）拼出窗口标题"<ISA>-NEMU"
  char title[128];
  sprintf(title, "%s-NEMU", str(__GUEST_ISA__));
  // 初始化视频子系统
  SDL_Init(SDL_INIT_VIDEO);
  // 创建窗口与渲染器。
  // 窗口尺寸基于逻辑分辨率 SCREEN_W/H，若选择 400x300 配置则放大 2 倍显示（更清晰）
  // 渲染器与窗口绑定，用于在该窗口上绘制
  SDL_CreateWindowAndRenderer(
      SCREEN_W * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      SCREEN_H * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      0, &window, &renderer);
  // 设置窗口标题
  SDL_SetWindowTitle(window, title);
  // 创建纹理对象，用于存储像素数据
  // 格式：SDL_PIXELFORMAT_ARGB8888（32 位 ARGB）
  // 访问模式：SDL_TEXTUREACCESS_STATIC（CPU 更新，GPU 读取）
  // 尺寸：SCREEN_W × SCREEN_H
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
  // 首次渲染提交, 创建窗口后需要一次 Present 才能让窗口可见
  // 清空显示：此时纹理为空，Present 后显示为黑色背景
  SDL_RenderPresent(renderer);
}

static inline void update_screen() {
  // SDL_UpdateTexture 将 vmem 的像素数据上传到纹理
  // pitch：SCREEN_W * sizeof(uint32_t)（每行字节数）
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint32_t));
  // 清空渲染目标
  SDL_RenderClear(renderer);
  // 将纹理复制到渲染器
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  // 提交渲染结果到窗口
  SDL_RenderPresent(renderer);
}
#else // CONFIG_TARGET_AM
static void init_screen() {}

static inline void update_screen() {
  io_write(AM_GPU_FBDRAW, 0, 0, vmem, screen_width(), screen_height(), true);
}
#endif 
#endif // CONFIG_VGA_SHOW_SCREEN

void vga_update_screen() {
  // TODO: call `update_screen()` when the sync register is non-zero,
  // then zero out the sync register
  if (vgactl_port_base[1] != 0) update_screen();
  vgactl_port_base[1] = 0;
}

void init_vga() {
  // 分配 8 字节（两个 32 位寄存器）
  vgactl_port_base = (uint32_t *)new_space(8);
  // vgactl_port_base[0] 存储分辨率：高 16 位：宽度，低 16 位：高度
  // 例如：400×300 → 0x0190012c（0x0190 = 400, 0x012c = 300）
  vgactl_port_base[0] = (screen_width() << 16) | screen_height();
  vgactl_port_base[1] = 0;
  // 映射到 PIO 或 MMIO 地址, 程序可读取分辨率信息
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("vgactl", CONFIG_VGA_CTL_PORT, vgactl_port_base, 8, NULL);
#else
  // 映射到 MMIO 地址 CONFIG_VGA_CTL_MMIO = 0xa0000100
  add_mmio_map("vgactl", CONFIG_VGA_CTL_MMIO, vgactl_port_base, 8, NULL);
#endif
  // 帧缓冲区分配 screen_size() 字节（宽 × 高 × 4字节/像素）
  vmem = new_space(screen_size());
  // 映射到 MMIO 地址 CONFIG_FB_ADDR = 0xa1000000
  add_mmio_map("vmem", CONFIG_FB_ADDR, vmem, screen_size(), NULL);
  IFDEF(CONFIG_VGA_SHOW_SCREEN, init_screen());
  IFDEF(CONFIG_VGA_SHOW_SCREEN, memset(vmem, 0, screen_size()));
}

/*
┌─────────────────────────────────────────────┐
│         SDL 渲染流程                         │
├─────────────────────────────────────────────┤
│                                             │
│  vmem (帧缓冲区)                             |
|  ├─ 分辨率: 400×300 (逻辑分辨率)             │
│  ├─ 存储位置: 主机内存                       │
│  └─ 内容: 像素数据 (ARGB8888)                │
│         ↓ SDL_UpdateTexture()               │
│  texture (纹理)                             |
|  ├─ 分辨率: 400×300 (与vmem一致)             │
│  ├─ 存储位置: GPU 内存（或主机内存）          │
│  └─ 内容: 像素数据 (ARGB8888)                │
│         ↓ SDL_RenderCopy()                  │
│  renderer (渲染器)                           |
|  ├─ 分辨率: 800×600 (显示分辨率)             │
│  ├─ 作用: 绘制到窗口                         │
│  └─ 目标: SDL 窗口                           │
│         ↓ SDL_RenderPresent()               │
│      屏幕显示                                │
│                                             │
└─────────────────────────────────────────────┘

原始像素 (400×300)          放大后 (800×600)
┌─────────────┐            ┌─────────────────┐
│  R  G  B    │            │  R R  G G  B B  │
│  R  G  B    │    →       │  R R  G G  B B  │
│  R  G  B    │            │  R R  G G  B B  │
└─────────────┘            │  R R  G G  B B  │
                           └─────────────────┘
*/