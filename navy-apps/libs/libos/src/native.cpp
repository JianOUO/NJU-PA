#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <dlfcn.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

//#define MODE_800x600

#define FPS 60
#define WINDOW_W 800
#define WINDOW_H 600
#ifdef MODE_800x600
const int disp_w = WINDOW_W, disp_h = WINDOW_H;
#else
const int disp_w = 400, disp_h = 300;
#endif
static int pipe_size = 0;
#define FB_SIZE (disp_w * disp_h * sizeof(uint32_t))
#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0x00000000

static FILE *(*glibc_fopen)(const char *path, const char *mode) = NULL;
static int (*glibc_open)(const char *path, int flags, ...) = NULL;
static ssize_t (*glibc_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*glibc_write)(int fd, const void *buf, size_t count) = NULL;
static int (*glibc_execve)(const char *filename, char *const argv[], char *const envp[]) = NULL;

static SDL_Window *window = NULL;
static SDL_Surface *surface = NULL;
static SDL_TimerID texture_timer = 0;
static SDL_Thread *event_thread_handle = NULL;
static int dummy_fd = -1;
static int dispinfo_fd = -1;
static int fb_memfd = -1;
static int evt_fd = -1;
static int sb_fifo[2] = {-1, -1};
static int sbctl_fd = -1;
static uint32_t *fb = NULL;
static char fsimg_path[512] = "";

static inline void get_fsimg_path(char *newpath, const char *path) {
  sprintf(newpath, "%s%s", fsimg_path, path);
}

#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define COND(k) \
  if (scancode == SDL_SCANCODE_##k) name = #k;

#define KEY_QUEUE_LEN 64
static SDL_Event key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;
static SDL_mutex *key_queue_lock = NULL;

static int event_thread(void *args) {
  SDL_Event event;
  while (1) {
    SDL_WaitEvent(&event);

    switch (event.type) {
      case SDL_QUIT: exit(0); break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        SDL_LockMutex(key_queue_lock);
        key_queue[key_r] = event;
        key_r = (key_r + 1) % KEY_QUEUE_LEN;
        assert(key_r != key_f);
        SDL_UnlockMutex(key_queue_lock);
        break;
    }
  }
  return 0;
}

// 这是一个定时器回调函数，定期将帧缓冲区（surface）的内容缩放并复制到 SDL 窗口表面，然后刷新到屏幕，实现双缓冲显示
static Uint32 texture_sync(Uint32 interval, void *param) {
// 缩放并复制（Blit）一个表面到另一个表面
// surface：源表面（帧缓冲区）尺寸：disp_w × disp_h（400×300 或 800×600）内容：程序绘制的像素数据 来源：从 fb（帧缓冲区）创建
// NULL：源矩形（NULL 表示整个源表面）
// SDL_GetWindowSurface(window)：目标表面（窗口表面）尺寸：WINDOW_W × WINDOW_H（800×600）
// NULL：目标矩形（NULL 表示整个目标表面）
  SDL_BlitScaled(surface, NULL, SDL_GetWindowSurface(window), NULL);
// 将窗口表面的内容刷新到屏幕
// SDL_BlitScaled 只在内存中操作，这一步才真正显示
  SDL_UpdateWindowSurface(window);
// 作用：返回相同的间隔，保持定时器继续运行
// 效果：定时器会继续以相同频率触发
  return interval;
}

static void audio_fill(void *userdata, uint8_t *stream, int len) {
  int nread = glibc_read(sb_fifo[0], stream, len);
  if (nread == -1) nread = 0;
  if (nread < len) memset(stream + nread, 0, len - nread);
}

static void open_display() {
  // memfd_create：Linux 系统调用，创建匿名内存文件描述符
  // "fb"：文件名（用于调试）
  // 0：标志（0 表示默认行为）
  // 返回值：文件描述符（成功）或 -1（失败）
  // 创建一个仅存在于内存的“文件”，不占用磁盘空间
  fb_memfd = memfd_create("fb", 0);
  assert(fb_memfd != -1);
  // 设置文件大小为FB_SIZE（帧缓冲区大小）
  int ret = ftruncate(fb_memfd, FB_SIZE);
  assert(ret == 0);
  // mmap：将文件映射到内存, NULL：让系统选择映射地址, FB_SIZE：映射大小, PROT_READ | PROT_WRITE：可读可写
  // MAP_SHARED：共享映射，多个进程可以映射到同一个文件, fb_memfd：文件描述符, 0：偏移量
  // 返回值：映射地址（成功）或 (void *)-1（失败）
  // 将内存文件映射到进程地址空间，可直接通过指针访问
  fb = (uint32_t *)mmap(NULL, FB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fb_memfd, 0);
  assert(fb != (void *)-1);
  // 将帧缓冲区初始化为黑色（全0）
  memset(fb, 0, FB_SIZE);
  // 将文件指针重置到开头，确保后续读写从开始位置进行
  lseek(fb_memfd, 0, SEEK_SET);

  SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  // 创建普通 SDL 窗口；这里只使用窗口表面做软件 blit，不需要 OpenGL 上下文
  window = SDL_CreateWindow("Simulated Nanos Application",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, 0);
  // 创建RGB表面，从fb指向的内存中创建，尺寸为disp_w × disp_h，32位深度，每行字节数为disp_w × sizeof(uint32_t)，掩码为RMASK, GMASK, BMASK, AMASK
  surface = SDL_CreateRGBSurfaceFrom(fb, disp_w, disp_h, 32, disp_w * sizeof(uint32_t),
      RMASK, GMASK, BMASK, AMASK);
  // 添加定时器，每隔 1000/FPS 毫秒调用一次 texture_sync 函数
  texture_timer = SDL_AddTimer(1000 / FPS, texture_sync, NULL);
}

static void open_event() {
  // 创建保护键盘事件队列的互斥锁
  key_queue_lock = SDL_CreateMutex();
/*
程序调用：
open("/dev/events", O_RDONLY)
    └─> 返回 evt_fd (dup(dummy_fd))
程序调用：
  read(evt_fd, buf, count)
    └─> 被拦截（第208行）
    └─> 从 key_queue 读取 SDL 键盘事件
    └─> 转换为文本格式返回
        "kd RETURN\n"  (按下回车)
        "ku A\n"       (松开A键)
*/
  evt_fd = dup(dummy_fd);
  // 互斥锁准备好后再启动事件线程，避免线程先收到按键事件时访问空锁
  event_thread_handle = SDL_CreateThread(event_thread, "event thread", nullptr);
}

static void open_audio() {
  // pipe2()：创建管道（pipe() + 标志）
  // sb_fifo[0]：读端, sb_fifo[1]：写端, O_NONBLOCK：非阻塞模式
  /*
  程序调用：
  open("/dev/sb", O_WRONLY)
    └─> 返回 sb_fifo[1] (管道写端)
  程序调用：
  write(sb_fifo[1], audio_data, len)
    └─> 写入管道
  SDL 音频回调：
  audio_fill() 
    └─> 从 sb_fifo[0] 读取音频数据
    └─> 填充到 SDL 音频流
  */
  int ret = pipe2(sb_fifo, O_NONBLOCK);
  assert(ret == 0);
  /*
  程序调用：
  open("/dev/sbctl", O_RDWR)
    └─> 返回 sbctl_fd (dup(dummy_fd))

  程序写入（初始化音频）：
  write(sbctl_fd, args, 12)
    └─> 被拦截（第240行）
    └─> 解析参数：freq, channels, samples
    └─> 初始化 SDL 音频

  程序读取（查询缓冲区）：
  read(sbctl_fd, buf, count)
    └─> 被拦截（第229行）
    └─> 返回管道空闲空间大小
  */
  sbctl_fd = dup(dummy_fd);
  // 获取管道大小
  pipe_size = fcntl(sb_fifo[0], F_GETPIPE_SZ);
}
/*
Nanos-lite:
程序请求：
  fopen("/share/music/song.ogg", "r")
操作系统处理：
  └─> 从 ramdisk.img 中读取文件
  └─> 文件系统已经挂载在根目录
  └─> 虚拟路径就是实际路径

Native 模式（本地 Linux）:
程序请求：
  fopen("/share/music/song.ogg", "r")
问题：
  └─> Linux 文件系统中不存在 /share/music/song.ogg
  └─> 文件实际在 $NAVY_HOME/fsimg/share/music/song.ogg
解决方案：
  └─> 路径重定向
  └─> 将虚拟路径转换为本地路径
*/
static const char* redirect_path(char *newpath, const char *path) {
  // 将虚拟路径转换为本地路径
  get_fsimg_path(newpath, path);
  // access()：检查文件是否存在和可访问, 0：F_OK，只检查存在性
  // 0 == access()：文件存在时返回 0
  if (0 == access(newpath, 0)) {
    fprintf(stderr, "Redirecting file open: %s -> %s\n", path, newpath);
    return newpath;
  }
  // 文件不存在时返回原始路径
  return path;
}

/*
C++ 编译器会对函数名进行修饰（mangling）：
  fopen() → _Z5fopenPKcS0_  (包含参数类型信息)

C 编译器不进行名称修饰：
  fopen() → fopen  (原始函数名)

程序调用 fopen()：
  └─> 链接器查找符号 "fopen"
  └─> C++ 编译的函数名是 "_Z5fopenPKcS0_"
  └─> 找不到匹配的符号
  └─> 链接失败或无法拦截

extern "C" 告诉编译器不要进行名称修饰，使用原始函数名, 这样链接器可以正确匹配符号，成功拦截 C 标准库函数
*/
extern "C" FILE *fopen(const char *path, const char *mode);
extern "C" int open(const char *path, int flags, ...);
extern "C" ssize_t read(int fd, void *buf, size_t count);
extern "C" ssize_t write(int fd, const void *buf, size_t count);
extern "C" int execve(const char *filename, char *const argv[], char *const envp[]);

// 这段代码使用函数拦截模式，通过 dlsym 和 RTLD_NEXT 获取 glibc 的原始 fopen
FILE *fopen(const char *path, const char *mode) {
  char newpath[512];
  if (glibc_fopen == NULL) {
    // RTLD_NEXT 特殊句柄: 在符号搜索顺序中，跳过当前库，查找下一个库中的符号
    /*
    程序启动：
    （1）加载动态库
      └─> 加载 native.cpp 编译的共享库
      └─> 加载 glibc 库

    （2）全局对象初始化
      └─> Init::Init() 构造函数执行
        └─> dlsym(RTLD_NEXT, "fopen")
           └─> 跳过当前库，查找 glibc
           └─> 返回 glibc 的 fopen 地址
           └─> 保存到 glibc_fopen

    （3）程序调用 fopen()
      └─> 链接器解析符号
          └─> 找到 native.cpp 中的 fopen()（优先级更高）
          └─> 调用拦截函数

    （4）拦截函数执行
      └─> 检查 glibc_fopen 是否已初始化
          └─> 如果未初始化，则初始化为glibc中的fopen
      └─> 调用 glibc_fopen()（真正的 glibc 函数）
          └─> 执行实际的文件打开操作
    */
    glibc_fopen = (FILE*(*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    assert(glibc_fopen != NULL);
  }
  /*
  为什么需要拦截？
  */
  return glibc_fopen(redirect_path(newpath, path), mode);
}

int open(const char *path, int flags, ...) {
  if (strcmp(path, "/proc/dispinfo") == 0) {
    return dispinfo_fd;
  } else if (strcmp(path, "/dev/events") == 0) {
    return evt_fd;
  } else if (strcmp(path, "/dev/fb") == 0) {
    return fb_memfd;
  } else if (strcmp(path, "/dev/sb") == 0) {
    return sb_fifo[1];
  } else if (strcmp(path, "/dev/sbctl") == 0) {
    return sbctl_fd;
  } else {
    char newpath[512];
    return glibc_open(redirect_path(newpath, path), flags);
  }
}

ssize_t read(int fd, void *buf, size_t count) {
  if (fd == dispinfo_fd) {
    // This does not strictly conform to `navy-apps/README.md`.
    // But it should be enough for real usage. Modify it if necessary.
    return snprintf((char *)buf, count, "WIDTH: %d\nHEIGHT: %d\n", disp_w, disp_h);
  } else if (fd == evt_fd) {
    int has_key = 0;
    SDL_Event ev = {};
    SDL_LockMutex(key_queue_lock);
    if (key_f != key_r) {
      ev = key_queue[key_f];
      key_f = (key_f + 1) % KEY_QUEUE_LEN;
      has_key = 1;
    }
    SDL_UnlockMutex(key_queue_lock);

    if (has_key) {
      SDL_Keysym k = ev.key.keysym;
      int keydown = ev.key.type == SDL_KEYDOWN;
      int scancode = k.scancode;

      const char *name = NULL;
      _KEYS(COND);
      if (name) return snprintf((char *)buf, count, "k%c %s\n", keydown ? 'd' : 'u', name);
    }
    return 0;
  } else if (fd == sbctl_fd) {
    // return the free space of sb_fifo
    int used;
    // ioctl()：设备控制操作, sb_fifo[0]：管道的读端文件描述符
    // FIONREAD：查询可读字节数(即已写入但未读取的字节数), &used：输出参数，返回已使用的字节数
    ioctl(sb_fifo[0], FIONREAD, &used);
    int free = pipe_size - used;
    return snprintf((char *)buf, count, "%d", free);
  }
  return glibc_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
  if (fd == sbctl_fd) {
    // open audio
    const int *args = (const int *)buf;
    assert(count >= sizeof(int) * 3);
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec spec = {0};
    spec.freq = args[0];
    spec.channels = args[1];
    spec.samples = args[2];
    spec.userdata = NULL;
    spec.callback = audio_fill;
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
    return count;
  }
  return glibc_write(fd, buf, count);
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
  char newpath[512];
  glibc_execve(redirect_path(newpath, filename), argv, envp);
  return -1;
}

struct Init {
  Init() {
    glibc_fopen = (FILE*(*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    assert(glibc_fopen != NULL);
    glibc_open = (int(*)(const char*, int, ...))dlsym(RTLD_NEXT, "open");
    assert(glibc_open != NULL);
    glibc_read = (ssize_t (*)(int fd, void *buf, size_t count))dlsym(RTLD_NEXT, "read");
    assert(glibc_read != NULL);
    glibc_write = (ssize_t (*)(int fd, const void *buf, size_t count))dlsym(RTLD_NEXT, "write");
    assert(glibc_write != NULL);
    glibc_execve = (int(*)(const char*, char *const [], char *const []))dlsym(RTLD_NEXT, "execve");
    assert(glibc_execve != NULL);

    dummy_fd = memfd_create("dummy", 0);
    assert(dummy_fd != -1);
    dispinfo_fd = dummy_fd;

    char *navyhome = getenv("NAVY_HOME");
    assert(navyhome);
    sprintf(fsimg_path, "%s/fsimg", navyhome);
    // 将 PATH 环境变量设置为$NAVY_HOME/fsimg/bin，使程序可以通过程序名(如 "hello")执行，而无需完整路径
    char newpath[512];
    get_fsimg_path(newpath, "/bin");
    setenv("PATH", newpath, 1); // overwrite the current PATH

    SDL_Init(0);
    if (!getenv("NWM_APP")) {
      open_display();
      open_event();
    }
    open_audio();
  }
  ~Init() {
    if (texture_timer != 0) {
      SDL_RemoveTimer(texture_timer);
      texture_timer = 0;
    }
    if (event_thread_handle != NULL) {
      SDL_Event quit = {};
      quit.type = SDL_QUIT;
      SDL_PushEvent(&quit);
      SDL_WaitThread(event_thread_handle, NULL);
      event_thread_handle = NULL;
    }
    if (surface != NULL) {
      SDL_FreeSurface(surface);
      surface = NULL;
    }
    if (window != NULL) {
      SDL_DestroyWindow(window);
      window = NULL;
    }
    if (key_queue_lock != NULL) {
      SDL_DestroyMutex(key_queue_lock);
      key_queue_lock = NULL;
    }
    if (fb != NULL) {
      munmap(fb, FB_SIZE);
      fb = NULL;
    }
    if (sb_fifo[0] != -1) {
      close(sb_fifo[0]);
      sb_fifo[0] = -1;
    }
    if (sb_fifo[1] != -1) {
      close(sb_fifo[1]);
      sb_fifo[1] = -1;
    }
    if (sbctl_fd != -1) {
      close(sbctl_fd);
      sbctl_fd = -1;
    }
    if (evt_fd != -1) {
      close(evt_fd);
      evt_fd = -1;
    }
    if (fb_memfd != -1) {
      close(fb_memfd);
      fb_memfd = -1;
    }
    if (dummy_fd != -1) {
      close(dummy_fd);
      dummy_fd = -1;
      dispinfo_fd = -1;
    }
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
      SDL_CloseAudio();
    }
    SDL_Quit();
  }
};

Init init;
