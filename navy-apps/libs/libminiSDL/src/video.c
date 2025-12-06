#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// 将一张画布中的指定矩形区域复制到另一张画布的指定位置
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BytesPerPixel == 1 || dst->format->BytesPerPixel == 4);
  assert(src->pixels && dst->pixels);
  int BytesPerPixel = dst->format->BytesPerPixel;
  int16_t dstrect_x = (dstrect == NULL ? 0 : dstrect->x); 
  int16_t dstrect_y = (dstrect == NULL ? 0 : dstrect->y);
  int16_t srcrect_x = (srcrect == NULL ? 0 : srcrect->x);
  int16_t srcrect_y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);
  int dst_offset = (dstrect_y * dst->w + dstrect_x);
  int src_offset = (srcrect_y * src->w + srcrect_x);
  if (w + srcrect_x > src->w) w = src->w - srcrect_x;
  if (w + dstrect_x > dst->w) w = dst->w - dstrect_x;
  if (h + srcrect_y > src->h) h = src->h - srcrect_y;
  if (h + dstrect_y > dst->h) h = dst->h - dstrect_y;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      int srcrect_offset = i * src->w + j;
      int dstrect_offset = i * dst->w + j;
      if (BytesPerPixel == 1) {
        dst->pixels[dst_offset + dstrect_offset] = src->pixels[src_offset + srcrect_offset];
      } else {
        ((uint32_t *)dst->pixels)[dst_offset + dstrect_offset] = ((uint32_t *)src->pixels)[src_offset + srcrect_offset];
      }
    }
  }
} 

// 用指定颜色填充表面的矩形区域
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
  assert(0);
}

// 将画布中的指定矩形区域同步到屏幕上
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  //assert(0);
  assert(s && s->format && s->pixels);
  assert(s->format->BitsPerPixel == 32);
  if (!x && !y && !w && !h) {
    w = s->w;
    h = s->h;
  }
  if (w + x > s->w) w = s->w - x;
  if (h + y > s->h) h = s->h - y;
  NDL_DrawRect((uint32_t *)s->pixels, x, y, w, h);
}

// APIs below are already implemented.

// 从颜色掩码计算颜色分量的位移量
static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  // 8 位模式（调色板模式）
  if (depth == 8) {
    // 分配调色板结构
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    // 分配256个颜色条目
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    // 将256个颜色初始化为0
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    // 设置调色板中的颜色数量为256
    s->format->palette->ncolors = 256;
  } else { // 32 位模式（真彩色）
    // 不使用调色板
    s->format->palette = NULL;
    // 设置 RGB 和 Alpha 的掩码, 位移量和精度损失(0, 无损失)
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  // 设置像素格式的基本属性
  // 每像素位数（8 或 32）
  s->format->BitsPerPixel = depth;
  // 每像素字节数（1 或 4）
  s->format->BytesPerPixel = depth / 8;

  // 设置表面宽度、高度和每行字节数
  s->w = width; // 表面宽度
  s->h = height; // 表面高度
  // 每行字节数 = 宽度 * 每像素位数 / 8
  s->pitch = width * depth / 8;
  // 确保每行字节数等于宽度 * 每像素字节数
  assert(s->pitch == width * s->format->BytesPerPixel);

  // 如果表面不是预分配的
  if (!(flags & SDL_PREALLOC)) {
    // 分配像素缓冲区为 pitch × height（总字节数）
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

// 释放表面内存
void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

// bpp: bits per pixel
SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
/*
硬件表面 vs 软件表面
特性	    SDL_HWSURFACE         SDL_SWSURFACE
内存位置	显存/帧缓冲区	          系统内存（RAM）
初始化	  调用 NDL_OpenCanvas()	 不调用
用途	    主显示窗口	           后备缓冲区、离屏渲染
性能	    可能更快（直接显示）	  CPU 渲染
*/
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

// SDL_SoftStretch 用于将源表面的矩形区域缩放并复制到目标表面。当前实现仅支持相同尺寸的复制，真正的缩放功能未实现。
void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  // 检查源和目标表面非空
  assert(src && dst);
  // 检查目标表面和源表面的像素格式相同
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  // 限制为 8 位（调色板模式）
  assert(dst->format->BitsPerPixel == 8);

  // 如果 srcrect 为 NULL，使用整个源表面（从 (0,0) 开始，尺寸为 src->w × src->h）
  // 否则使用 srcrect 指定的区域
  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  // 检查目标矩形非空
  assert(dstrect);
  // 如果源和目标尺寸相同，直接调用 SDL_BlitSurface 复制（无需缩放）
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else { // 如果尺寸不同，触发断言失败（缩放功能未实现）
    assert(0);
  }
}

/*
在 8 位模式下：
每个像素是一个 0-255 的索引值
索引值对应调色板中的实际颜色
调色板包含 256 个 RGB 颜色
像素数据：       调色板：
[0x42]  ──→  colors[0x42] = {r:255, g:128, b:0}  (橙色)
[0x00]  ──→  colors[0x00] = {r:0,   g:0,   b:0}  (黑色)
[0xFF]  ──→  colors[0xFF] = {r:255, g:255, b:255} (白色)
*/
// SDL_SetPalette 用于设置 8 位颜色模式表面的调色板，将颜色索引映射到实际 RGB 颜色
void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  // 检查表面、格式和调色板指针非空
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  // 限制 firstcolor 必须为 0（不支持部分更新）
  assert(firstcolor == 0);

  // 设置调色板中的颜色数量
  s->format->palette->ncolors = ncolors;
  // 将颜色数据复制到调色板中
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  // 如果是硬件表面：
  if(s->flags & SDL_HWSURFACE) {
    // 要求颜色数量为 256 
    assert(ncolors == 256);
    // 循环提取 RGB 值（当前未使用，可能是未完成实现）
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    // 调用 SDL_UpdateRect 刷新显示
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

// dst：目标缓冲区（ABGR 格式）src：源缓冲区（ARGB 格式）len：像素数量
/*
在小端序系统中，32 位像素在内存中的字节顺序（低地址 → 高地址）：
ARGB 格式：
字节0: B (蓝色) 字节1: G (绿色) 字节2: R (红色) 字节3: A (Alpha)
ABGR 格式：
字节0: R (红色) 字节1: G (绿色) 字节2: B (蓝色) 字节3: A (Alpha)
转换需要：交换字节 0 和字节 2（R ↔ B），G 和 A 保持不变。
*/
static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  // 数组指针声明 uint8_t (*pdst)[4]：指向 4 字节数组的指针
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  // 用于同时以 32 位值和字节数组访问同一数据
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  // 向下对齐到 16 的倍数, 用于批量处理 16 个像素为一组
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
/*
tmp.val32 = *((uint32_t *)psrc[i])：读取源像素（ARGB）
*((uint32_t *)pdst[i]) = tmp.val32：先复制整个 32 位值
pdst[i][0] = tmp.val8[2]：将字节 2（R）放到位置 0
pdst[i][2] = tmp.val8[0]：将字节 0（B）放到位置 2
*/
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];
  // 循环展开：每次处理 16 个像素，减少循环开销。
    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }
  // 处理剩余的像素（不足 16 个）
  for (; i < len; i ++) {
    macro(i);
  }
}

// src：源表面 fmt：目标像素格式 flags：标志位
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  // 源表面必须是 32 位
  assert(src->format->BitsPerPixel == 32);
  // 验证 pitch 计算正确（无填充）
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  // 源和目标位深度必须相同（32 位）
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);
  // 创建与源相同尺寸的新表面, 使用目标格式的掩码
  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  // 绿色掩码必须相同（G 位置不变）
  assert(fmt->Gmask == src->format->Gmask);
  // Alpha 掩码：要么都为 0（无 Alpha），要么相同
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  // 调用转换函数：将源表面的 ARGB 格式转换为目标表面的 ABGR 格式
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

//  将RGBA 颜色值（0-255）转换为符合指定像素格式的 32 位像素值。
uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  assert(0);
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
  assert(0);
}
