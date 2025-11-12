#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <klib.h>
#include <SDL.h>

static int rfd = -1, wfd = -1;
// 管道中待播放的音频数据字节数
// volatile - 告诉编译器 count 可能被其他线程修改, 每次访问都从内存读取，不缓存在寄存器中
static volatile int count = 0;

void __am_audio_init() {
  int fds[2];
  int ret = pipe2(fds, O_NONBLOCK);
  assert(ret == 0);
  rfd = fds[0];
  wfd = fds[1];
}

// userdata - 用户自定义数据（本例中未使用）
// stream - SDL 提供的输出缓冲区，需要填充音频数据
// len - SDL 请求的字节数（需要填充的数据量）
// SDL 每隔一段时间（根据采样率和缓冲区大小）调用一次, 用于及时填充数据,否则会出现"爆音"或"卡顿"
static void audio_play(void *userdata, uint8_t *stream, int len) {
  int nread = len;
  if (count < len) nread = count;
  int b = 0;
  while (b < nread) {
    int n = read(rfd, stream, nread);
    if (n > 0) b += n;
  }

  count -= nread;
  // 剩余的 len - nread 字节用0填充（静音）,避免播放未初始化的数据（噪音）
  if (len > nread) {
    memset(stream + nread, 0, len - nread);
  }
}

// 生产者端：将数据写入管道
static void audio_write(uint8_t *buf, int len) {
  int nwrite = 0;
  while (nwrite < len) {
    int n = write(wfd, buf, len); // int n = write(wfd, buf + nwrite, len - nwrite);
    if (n == -1) n = 0; // 如果管道满了，write() 立即返回 -1
    count += n;
    nwrite += n;
  }
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  // SDL_AudioSpec - SDL 的音频规格结构体
  // {} - C99 初始化语法，将所有字段初始化为 0/NULL
  SDL_AudioSpec s = {};
  // 设置采样率(每秒采集音频样本的次数,单位：Hz)
  // 44100 Hz - CD 音质标准, 48000 Hz - DVD、专业音频标准, 8000 Hz - 电话质量
  s.freq = ctrl->freq;
  // AUDIO_S16SYS - 16位有符号整数，系统字节序(自动选择大端或小端)
  // 每个音频样本用 16 位（-32768 到 32767）表示,0 表示静音,正值表示正压力，负值表示负压力
  s.format = AUDIO_S16SYS;
  // 声道数，1 表示单声道，2 表示立体声
  s.channels = ctrl->channels;
  // samples - 音频缓冲区大小（样本数）
  // 这是每次回调请求的样本数（不是字节数）,通常是 2 的幂：512, 1024
  s.samples = ctrl->samples;
  // audio_play - 之前定义的回调函数,SDL 会在音频线程中定期调用这个函数,每次调用时需要填充一个缓冲区的音频数据
  s.callback = audio_play;
  // 传递给回调函数的用户自定义数据,本例中不需要，设为 NULL
  s.userdata = NULL;

  // 清空管道中的数据计数,准备开始新的播放会话
  count = 0;
  // 初始化 SDL 音频子系统
  // SDL_InitSubSystem() - 初始化 SDL 的特定子系统, SDL_INIT_AUDIO - 只初始化音频子系统
  int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
  // 如果初始化成功，打开音频设备并开始播放
  if (ret == 0) {
    // 第一个参数：期望的音频规格,第二个参数：实际获得的规格(如果硬件不支持期望的参数，SDL 会自动转换,传入 NULL 表示信任 SDL 的选择)
    SDL_OpenAudio(&s, NULL);
    // 控制音频播放状态,0 表示播放,1 表示暂停
    SDL_PauseAudio(0);
  }
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = count; // stat->count - 管道中待播放的音频数据字节数
}

// 将音频数据写入播放缓冲区
void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int len = ctl->buf.end - ctl->buf.start;
  audio_write(ctl->buf.start, len);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  // 表示音频设备可用
  cfg->present = true;
  // fcntl():文件控制操作, F_GETPIPE_SZ：获取管道的缓冲区大小
  cfg->bufsize = fcntl(rfd, F_GETPIPE_SZ);
}
