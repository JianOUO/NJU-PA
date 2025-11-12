#include <am.h>
#include <nemu.h>

// 采样率寄存器
#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
// 声道数寄存器
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
// 缓冲区大小寄存器(样本数)，配置每次中断的样本数
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
// 流缓冲区大小寄存器(字节)，控制音频数据的缓冲容量
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
// 初始化控制寄存器,触发音频设备初始化
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
// 待播放计数器,读取流缓冲区中待播放的字节数
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

void __am_audio_init() {
}

// 音频设备不存在
void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = false;
}

// 设置音频设备参数
void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
}

// 查询音频缓冲区状态
void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = 0; // 缓冲区中 0 字节待播放数据
}

// 播放音频数据
void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
}
