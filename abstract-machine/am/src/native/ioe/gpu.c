#include <am.h>
#include <SDL.h>
#include <fenv.h>

//#define MODE_800x600
#define WINDOW_W 800
#define WINDOW_H 600
#ifdef MODE_800x600
const int disp_w = WINDOW_W, disp_h = WINDOW_H;
#else
const int disp_w = 400, disp_h = 300;
#endif

#define FPS   60
/*
32位像素: [31:24] [23:16] [15:8] [7:0]
掩码含义:    AA      RR     GG    BB
RMASK = 0x00ff0000  →  [23:16] 红色
GMASK = 0x0000ff00  →  [15:8]  绿色
BMASK = 0x000000ff  →  [7:0]   蓝色
AMASK = 0x00000000  →  无Alpha（不透明）
*/
#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0x00000000

static SDL_Window *window = NULL;
static SDL_Surface *surface = NULL;

// 定期将后备缓冲区的内容刷新到窗口显示（双缓冲刷新）
// 参数 1：Uint32 interval - 当前定时器间隔（毫秒）
// 参数 2：void *param - 用户自定义数据
// 返回值：Uint32 - 下次触发的间隔, 返回 interval 表示保持相同间隔,返回 0 表示取消定时器（不再触发）
static Uint32 texture_sync(Uint32 interval, void *param) {
  // 缩放并拷贝表面（Surface Blitting with Scaling）
  // surface - 源表面（后备缓冲区）,尺寸：disp_w × disp_h（400×300 或 800×600）,程序绘制的内容都在这里
  // NULL - 源矩形（NULL 表示整个表面）
  // SDL_GetWindowSurface(window) - 目标表面（窗口表面）
  // NULL - 目标矩形（NULL 表示整个表面）
  SDL_BlitScaled(surface, NULL, SDL_GetWindowSurface(window), NULL);
  // 将窗口表面的内容刷新到屏幕,之前的 SDL_BlitScaled 只是在内存中操作,这一步才真正显示到屏幕上
  /*
  1. 程序绘制 → surface (后备缓冲)
  2. SDL_BlitScaled → window surface (前台缓冲)
  3. SDL_UpdateWindowSurface → 显示器
  */
  SDL_UpdateWindowSurface(window);
  return interval;
}

void __am_gpu_init() {
  // SDL_Init() - 初始化 SDL 库
  // SDL_INIT_VIDEO - 视频子系统, SDL_INIT_TIMER - 定时器子系统
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  // "Native Application" - 窗口标题
  // SDL_WINDOWPOS_UNDEFINED - 窗口位置, 让操作系统自动决定窗口位置,通常是屏幕中心或默认位置
  // WINDOW_W, WINDOW_H - 窗口尺寸
  // SDL_WINDOW_OPENGL - 窗口标志,创建支持 OpenGL 的窗口
  window = SDL_CreateWindow("Native Application",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_W, WINDOW_H, SDL_WINDOW_OPENGL);
  // SDL_SWSURFACE - 软件表面标志,表面存储在系统内存（RAM）中,使用 CPU 进行渲染（软件渲染）
  // 对比：SDL_HWSURFACE（硬件表面，在显存中）
  // disp_w, disp_h - 显示分辨率 默认：400 × 300 像素,这可能与窗口尺寸不同！
  // 32 - 位深度（Bits Per Pixel）,每个像素 32 位（4 字节）,格式为AGGB，每个颜色分量 8 位（256 级）
  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, disp_w, disp_h, 32,
      RMASK, GMASK, BMASK, AMASK);
  // 1000 / FPS - 定时器间隔(毫秒),每隔 1000/FPS 毫秒刷新一次
  // texture_sync - 回调函数指针
  // NULL - 用户自定义数据（本例中未使用）
  SDL_AddTimer(1000 / FPS, texture_sync, NULL);
}

// 作用：返回 GPU 设备的配置信息
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, // 表示 GPU 设备可用
    .has_accel = false, // 表示没有硬件加速支持,当前实现使用 CPU 软件渲染,不使用 GPU 硬件加速
    .width = disp_w, // 显示宽度 400 像素
    .height = disp_h, // 显示高度 300 像素
    .vmemsz = 0 // Native 平台使用系统内存（RAM）作为显存,不是专用的显存(VRAM),设为 0 表示"不适用"或"动态分配"
  };
}

void __am_gpu_status(AM_GPU_STATUS_T *stat) {
  stat->ready = true;
}

// 将用户提供的像素数据绘制到后备缓冲区
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // x, y - 在 surface 上的绘制位置（左上角坐标）
  // w, h - 要绘制的矩形尺寸（宽度和高度）
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  // 如果宽度或高度为 0，无需绘制
  if (w == 0 || h == 0) return;
  // feclearexcept() - 清除浮点异常标志
  // -1 在二进制中是全 1：0xFFFFFFFF, 相当于清除所有浮点异常标志
  // SDL 的某些操作可能涉及浮点运算,清除之前可能遗留的浮点异常,防止异常影响后续操作
  feclearexcept(-1);
  // SDL_CreateRGBSurfaceFrom() - 从现有像素数据创建表面（不复制）
  // ctl->pixels - 像素数据指针,指向用户提供的像素数组,格式：uint32_t pixels[],不会复制数据，直接使用这个指针
  // w, h - 图像尺寸, 宽度和高度（像素）
  // 32 - 位深度,每个像素 32 位（4 字节）,格式：ARGB
  // w * sizeof(uint32_t) - 行跨距（Pitch）,一行像素占用的字节数
  SDL_Surface *s = SDL_CreateRGBSurfaceFrom(ctl->pixels, w, h, 32, w * sizeof(uint32_t),
      RMASK, GMASK, BMASK, AMASK);
  // SDL_Rect - 矩形结构体,用于指定绘制位置和尺寸
  // .x = x, .y = y - 绘制位置（左上角坐标）
  // .w 和 .h 自动为 0（未使用）,因为SDL_BlitSurface 会使用源表面的尺寸
  SDL_Rect rect = { .x = x, .y = y };
  // s - 源表面,刚创建的临时表面,包含用户提供的像素数据
  // NULL - 源矩形,NULL表示复制整个源表面, 相当于 {0, 0, w, h}
  // surface - 目标表面(后备缓冲),全局变量，400×300的后备缓冲区,所有绘制操作的目标
  // &rect - 目标位置,指定在 surface 上的 (x, y) 位置, 源图像会被绘制到这里
  SDL_BlitSurface(s, NULL, surface, &rect);
  // SDL_CreateRGBSurfaceFrom 创建的表面必须手动释放
  // 只释放表面结构，不释放像素数据,像素数据 (ctl->pixels) 仍然由用户管理
  SDL_FreeSurface(s);
}
