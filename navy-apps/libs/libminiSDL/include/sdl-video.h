#ifndef __SDL_VIDEO_H__
#define __SDL_VIDEO_H__

// 这些是用于 SDL_Surface 的标志位，通过位掩码组合使用
// 硬件表面，使用显存（VRAM）。在 libminiSDL 中，设置此标志会调用 NDL_OpenCanvas() 打开画布
#define SDL_HWSURFACE 0x1
// 物理调色板，用于 8 位颜色模式
#define SDL_PHYSPAL 0x2
// 逻辑调色板，用于 8 位颜色模式
#define SDL_LOGPAL 0x4
// 软件表面，使用系统内存（RAM），由 CPU 渲染
#define SDL_SWSURFACE  0x8
// 预分配内存，表示像素缓冲区已由外部分配(如 SDL_CreateRGBSurfaceFrom())
#define SDL_PREALLOC  0x10
// 全屏模式
#define SDL_FULLSCREEN 0x20
// 窗口可调整大小
#define SDL_RESIZABLE  0x40

// 默认颜色掩码（Color Masks）
// 像素格式：ARGB（Alpha-Red-Green-Blue）
#define DEFAULT_RMASK 0x00ff0000
#define DEFAULT_GMASK 0x0000ff00
#define DEFAULT_BMASK 0x000000ff
#define DEFAULT_AMASK 0xff000000

// 表示屏幕上的矩形区域
// x, y（int16_t）：矩形左上角坐标（可为负）
// w, h（uint16_t）：宽度和高度（非负）
typedef struct {
	int16_t x, y;
	uint16_t w, h;
} SDL_Rect;

// 表示 RGBA 颜色，使用 union 提供两种访问方式。
/*
低地址 ──→ 高地址
+------+------+------+------+
| 0x12 | 0x34 | 0x56 | 0x78 |   // 按字节查看
+------+------+------+------+
   b       g     r      a

作为 uint32_t 查看：
color.val = 0x78563412
*/
typedef union {
  struct {
    //uint8_t r, g, b, a; 源码中原来的排序, 与上述的颜色掩码不符
	uint8_t b, g, r, a;
  };
  uint32_t val;
} SDL_Color;

typedef struct {
	int ncolors; // 调色板中的颜色数量
	SDL_Color *colors; // 调色板中的颜色数组
} SDL_Palette;

// 描述像素的存储格式，用于颜色转换和像素操作
/*
像素格式示例（32 位 ARGB）：
假设像素值：0xFF123456
Rmask = 0x00ff0000, Rshift = 16
提取红色分量：
uint32_t pixel = 0xFF123456;
uint8_t red = (pixel & format->Rmask) >> format->Rshift;
red = (0xFF123456 & 0x00ff0000) >> 16 = 0x12
*/
typedef struct {
	// 8 位模式时使用，32 位模式为 NULL
	SDL_Palette *palette;
	// 每像素位数（8 或 32）
	uint8_t BitsPerPixel;
	// 每像素字节数（1 或 4）
	uint8_t BytesPerPixel;
	// 颜色分量精度损失（通常为 0）
	uint8_t Rloss, Gloss, Bloss, Aloss;
	// 颜色分量在像素中的位移量
	uint8_t Rshift, Gshift, Bshift, Ashift;
	// 颜色分量的位掩码
	uint32_t Rmask, Gmask, Bmask, Amask;
} SDL_PixelFormat;

typedef struct {
	// 标志位
	uint32_t flags;
	// 像素格式
	SDL_PixelFormat *format;
	// 宽度和高度（像素）
	int w, h;
	// 每行字节数
	uint16_t pitch;
	// 像素数据指针
	uint8_t *pixels;
} SDL_Surface;

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags);
void SDL_FreeSurface(SDL_Surface *s);
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color);
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h);
void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors);
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags);
uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int SDL_LockSurface(SDL_Surface *s);
void SDL_UnlockSurface(SDL_Surface *s);

#endif
