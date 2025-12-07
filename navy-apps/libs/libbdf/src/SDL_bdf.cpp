#include <SDL_bdf.h>
#include <stdlib.h>
#include <assert.h>

// 将 BDF 字体中的字符位图转换为 SDL_Surface，用于渲染
// font: BDF 字体对象指针 ch: 要渲染的字符（ASCII 码）
// fg: 前景色（字符像素颜色）bg: 背景色（空白区域颜色）
SDL_Surface* BDF_CreateSurface(BDF_Font *font, char ch, uint32_t fg, uint32_t bg) {
  // 获取字符 ch 的位图数组（uint32_t[h]）
  // bm[j] 表示第 j 行的位图数据
  //printf("ch:%d %c\n", ch, ch);
  uint32_t *bm = font->font[ch];
  if (!bm) return NULL;
  // w, h: 字符的统一尺寸（宽度和高度）
  int w = font->w, h = font->h;
  // pixels: 像素数组，大小为 w × h，每个像素是一个 uint32_t 颜色值
  uint32_t *pixels = (uint32_t *)malloc(w * h * sizeof(uint32_t));
  assert(pixels);
  // j: 行索引（0 到 h-1）
  for (int j = 0; j < h; j ++) {
    // i: 列索引（0 到 w-1），从左到右
    for (int i = 0; i < w; i ++) {
      // bm[j] >> i: 将第 j 行的第 i 位右移到最低位 
      // & 1: 提取该位的值（0 或 1）
      // ? fg : bg: 若为 1 则前景色，否则背景色
      pixels[j * w + i] = ((bm[j] >> i) & 1) ? fg : bg;
    }
  }
  // 从已有的像素数据创建 SDL_Surface
  SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, w * sizeof(uint32_t),
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
  // 清除 SDL_PREALLOC 标志, 告诉 SDL 不要假设像素数据是预分配的，释放 Surface 时应释放像素内存
  s->flags &= ~SDL_PREALLOC;
  return s;
}
