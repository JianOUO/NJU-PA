/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq, // 采样频率（Hz）
  reg_channels, // 声道数
  reg_samples, // 每次中断处理的样本数
  reg_sbuf_size, // 流缓冲区大小(字节)
  reg_init, // 初始化控制寄存器,触发音频设备初始化
  reg_count, // 待播放计数器,读取流缓冲区中待播放的字节数
  nr_reg // 寄存器数量
};

// 指向音频流缓冲区的起始地址
static uint8_t *sbuf = NULL;
// 指向音频控制寄存器数组的起始地址
static uint32_t *audio_base = NULL;

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
}

// 初始化音频设备，分配控制寄存器和流缓冲区，并建立内存映射。
void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
  // 如果支持端口 I/O（x86）：使用 add_pio_map, 地址：CONFIG_AUDIO_CTL_PORT = 0x200
  // 如果支持 MMIO（riscv, mips）：使用 add_mmio_map, 地址：CONFIG_AUDIO_CTL_MMIO = 0xa0000200
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
  // 分配流缓冲区，大小：CONFIG_SB_SIZE = 0x10000(64KB)
  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  // 建立流缓冲区的内存映射，地址：CONFIG_SB_ADDR = 0xa1200000
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}

/*
初始化流程：
init_audio()
  ├─> 计算控制寄存器空间大小（24 字节）
  ├─> 分配控制寄存器内存
  ├─> 映射控制寄存器到物理地址（PIO 或 MMIO）
  ├─> 分配流缓冲区（64KB）
  └─> 映射流缓冲区到物理地址（MMIO）

运行时访问：
CPU 访问控制寄存器
  └─> 通过 audio_base[reg_xxx] 访问
      └─> 触发 audio_io_handler() 回调

CPU 访问流缓冲区
  └─> 通过 sbuf[offset] 访问
      └─> 直接读写音频数据

这为音频设备提供了：
控制接口：通过寄存器配置音频参数
数据接口：通过流缓冲区传输音频数据
*/