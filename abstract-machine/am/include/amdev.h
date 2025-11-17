#ifndef __AMDEV_H__
#define __AMDEV_H__

// **MAY SUBJECT TO CHANGE IN THE FUTURE**

#define AM_DEVREG(id, reg, perm, ...) \
  enum { AM_##reg = (id) }; \
  typedef struct { __VA_ARGS__; } AM_##reg##_T;

/*
这些寄存器提供了统一的设备抽象接口：
统一访问方式：通过 ioe_read() 和 ioe_write() 访问
平台无关：不同平台实现不同，但接口一致
*/
AM_DEVREG( 1, UART_CONFIG,  RD, bool present); // 查询 UART 是否可用, present表示是否可用
AM_DEVREG( 2, UART_TX,      WR, char data); // 发送一个字符, data表示要发送的字符
AM_DEVREG( 3, UART_RX,      RD, char data); // 接收一个字符, data表示接收到的字符
AM_DEVREG( 4, TIMER_CONFIG, RD, bool present, has_rtc); // 查询定时器配置, present表示是否可用, has_rtc表示是否支持RTC
AM_DEVREG( 5, TIMER_RTC,    RD, int year, month, day, hour, minute, second); // 读取实时时钟（RTC）, 年月日时分秒
AM_DEVREG( 6, TIMER_UPTIME, RD, uint64_t us); // 读取系统运行时间, us表示运行时间（以微秒为单位）
AM_DEVREG( 7, INPUT_CONFIG, RD, bool present); // 查询输入设备是否可用, present表示是否可用
AM_DEVREG( 8, INPUT_KEYBRD, RD, bool keydown; int keycode); // 读取键盘事件, keydown表示是否按下, keycode表示按键码
AM_DEVREG( 9, GPU_CONFIG,   RD, bool present, has_accel; int width, height, vmemsz); // 查询GPU配置, present表示是否可用, has_accel表示是否支持加速, width表示宽度, height表示高度, vmemsz表示帧缓冲区(显存)大小
AM_DEVREG(10, GPU_STATUS,   RD, bool ready); // 查询 GPU 是否就绪
AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync); // 绘制矩形到帧缓冲区, x, y（位置）、pixels（像素数据）、w, h（宽高）、sync（是否同步）
AM_DEVREG(12, GPU_MEMCPY,   WR, uint32_t dest; void *src; int size); // 复制数据到显存, dest表示目标地址, src表示源地址, size表示复制大小
AM_DEVREG(13, GPU_RENDER,   WR, uint32_t root); // 渲染场景树, root表示根节点
AM_DEVREG(14, AUDIO_CONFIG, RD, bool present; int bufsize); // 查询音频配置, present表示是否可用, bufsize表示缓冲区大小
AM_DEVREG(15, AUDIO_CTRL,   WR, int freq, channels, samples); // 设置音频参数, freq表示频率, channels表示声道数, samples表示每帧采样数
AM_DEVREG(16, AUDIO_STATUS, RD, int count); // 查询音频缓冲区状态, count表示缓冲区中待播放的字节数
AM_DEVREG(17, AUDIO_PLAY,   WR, Area buf); // 播放音频数据, buf表示音频数据区域
AM_DEVREG(18, DISK_CONFIG,  RD, bool present; int blksz, blkcnt); // 查询磁盘配置, present表示是否可用, blksz表示块大小, blkcnt表示块数量
AM_DEVREG(19, DISK_STATUS,  RD, bool ready); // 查询磁盘是否就绪
AM_DEVREG(20, DISK_BLKIO,   WR, bool write; void *buf; int blkno, blkcnt); // 执行块 I/O 操作, write表示是否写入, buf表示数据缓冲区, blkno表示块号, blkcnt表示块数量
AM_DEVREG(21, NET_CONFIG,   RD, bool present); // 查询网络配置, present表示是否可用
AM_DEVREG(22, NET_STATUS,   RD, int rx_len, tx_len); // 查询网络状态, rx_len表示接收队列长度, tx_len表示发送队列长度
AM_DEVREG(23, NET_TX,       WR, Area buf); // 发送网络数据包, buf表示数据包区域
AM_DEVREG(24, NET_RX,       WR, Area buf); // 接收网络数据包, buf表示接受缓冲区区域

// Input

#define AM_KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define AM_KEY_NAMES(key) AM_KEY_##key,
enum {
  AM_KEY_NONE = 0,
  AM_KEYS(AM_KEY_NAMES)
};

// GPU

// 叶子节点，表示纹理（图片）
#define AM_GPU_TEXTURE  1
// 内部节点，表示包含子节点的子树
#define AM_GPU_SUBTREE  2
// 空指针值，表示没有子节点或兄弟节点
#define AM_GPU_NULL     0xffffffff

// gpuptr_t：GPU 内存中的偏移量（相对于显存基址）
typedef uint32_t gpuptr_t;

// 纹理描述结构
struct gpu_texturedesc {
  uint16_t w, h; // 纹理宽度和高度
  gpuptr_t pixels; // 像素数据在 GPU 内存中的地址
} __attribute__((packed)); // 紧凑布局，无填充

// 画布节点结构
struct gpu_canvas {
  // 节点类型（AM_GPU_TEXTURE 或 AM_GPU_SUBTREE）
  // w, h：画布逻辑尺寸, x1, y1：在父画布中的位置, w1, h1：在父画布中占用的显示尺寸
  uint16_t type, w, h, x1, y1, w1, h1;
  // 兄弟节点的指针（链表）
  gpuptr_t sibling;
  // type == AM_GPU_TEXTURE：使用 texture（纹理描述）
  // type == AM_GPU_SUBTREE：使用 child（第一个子节点）
  union {
    gpuptr_t child;
    struct gpu_texturedesc texture;
  };
} __attribute__((packed));
#endif

/*
                    Root Canvas (display)
                           |
                    ┌──────┴──────┐
                    |             |
                 SUBTREE         SUBTREE
                 (背景层)          (UI层)
                    |                |
            ┌───────┼───────┐    ┌───┴───┐
            |       |       |    |       |
        TEXTURE TEXTURE TEXTURE TEXTURE TEXTURE
         (天空)  (地面)  (云朵)  (按钮)  (文字)
*/
