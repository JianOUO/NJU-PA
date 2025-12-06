#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;
static struct timeval start;
static int dispinfo_fd = -1;
static int evt_fd = -1;
static int fb_memfd = -1;

uint32_t NDL_GetTicks() {
  struct timeval current;
  assert(gettimeofday(&current, 0) != -1);
  uint32_t elapsed_ms = (current.tv_sec - start.tv_sec) * 1000 + 
                        (current.tv_usec - start.tv_usec) / 1000;
  return elapsed_ms;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", O_RDONLY);
  assert(fd >= 0);
  ssize_t ret = read(fd, buf, len);
  assert(ret != -1);
  //if (ret != 0) printf("%s\n", buf);
  //close(fd); 不需要关闭特殊文件，否则会影响相关的文件操作
  return ret == 0 ? 0 : 1;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }

  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  assert(*w > 0 && *w <= screen_w);
  assert(*h > 0 && *h <= screen_h);
  canvas_w = *w;
  canvas_h = *h;
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;
  // 画布必须在屏幕内
  assert(canvas_x >= 0 && canvas_y >= 0 && canvas_y + canvas_h <= screen_h && canvas_x + canvas_w <= screen_w);
  printf("screen_w: %d screen_h: %d\n", screen_w, screen_h);
  printf("canvas_x: %d canvas_y: %d\n", canvas_x, canvas_y);
  printf("canvas_w: %d canvas_h: %d\n", canvas_w, canvas_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  // 矩形必须在画布内
  printf("1");
  assert(x >= 0 && y >= 0 && y + h <= canvas_h && x + w <= canvas_w);;
  int fd = open("/dev/fb", O_RDWR); 
  assert(fd != -1);
  int offset = (canvas_y + y) * screen_w + canvas_x + x;
  for (int i = 0; i < h; i++) {
    assert(lseek(fd, (offset + i * screen_w) * 4, SEEK_SET) != -1);
    assert(write(fd, pixels + i * w, w * 4) != -1);
  }
  //close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  assert(gettimeofday(&start, 0) != -1);

  int fd = open("/proc/dispinfo", O_RDONLY);
  assert(fd >= 0);
  char buf[64];
  assert(read(fd, buf, sizeof(buf)) >= 19);
  assert(sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h) == 2);
  //close(fd);

  return 0;
}

void NDL_Quit() {
}
