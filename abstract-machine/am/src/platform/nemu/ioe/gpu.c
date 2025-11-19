#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  /*
  int i;
  int w = io_read(AM_GPU_CONFIG).width;  
  int h = io_read(AM_GPU_CONFIG).height;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);
  */
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  int height = inw(VGACTL_ADDR);
  int width  = inw(VGACTL_ADDR + 2);

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = width * height * 4
  };
}

//AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync); 
//AM帧缓冲控制器, 可写入绘图信息, 向屏幕(x, y)坐标处绘制w*h的矩形图像. 图像像素按行优先方式存储在pixels中, 每个像素用32位整数以00RRGGBB的方式描述颜色. 若sync为true, 则马上将帧缓冲中的内容同步到屏幕上.
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  //int width = io_read(AM_GPU_CONFIG).width;  
  //int height = io_read(AM_GPU_CONFIG).height;
  int height = inw(VGACTL_ADDR);
  int width  = inw(VGACTL_ADDR + 2);
  int x = ctl->x;
  int y = ctl->y;
  uint32_t *pixels = (uint32_t *)ctl->pixels;
  if (y >= height || x >= width || y < 0 || x < 0) panic("(x,y) out of size"); 
  uint32_t *start = (uint32_t *)(uintptr_t)FB_ADDR + y * width + x;
  for (int j = 0; j < ctl->h; j++) {
    if (y + j >= height) break;
    for (int i = 0; i < ctl->w; i++) {
      if (x + i >= width) break;
      *(start + j * width + i) = pixels[j * ctl->w + i];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
