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

#ifndef __DEVICE_MAP_H__
#define __DEVICE_MAP_H__

#include <cpu/difftest.h>

// 用于 I/O 映射的回调，在访问设备内存时触发
// 参数分别为offset, len, is_write
typedef void(*io_callback_t)(uint32_t, int, bool);
// 分配并返回大小为 size 的设备内存空间指针
uint8_t* new_space(int size);

// IOMap 用于描述一个设备的内存映射区域，将物理地址映射到设备内存空间。
typedef struct {
  // 设备名称
  const char *name;
  // we treat ioaddr_t as paddr_t here
  paddr_t low; // 映射区域的起始物理地址
  paddr_t high; // 映射区域的结束物理地址
  // 指向设备内存空间的指针, 通常由new_space()分配, 实际数据存储位置
  void *space;
  // 访问该区域时触发的回调函数
  io_callback_t callback;
} IOMap;

// 检查访问地址是否在映射区域内
static inline bool map_inside(IOMap *map, paddr_t addr) {
  return (addr >= map->low && addr <= map->high);
}

/*
根据物理地址定位对应的设备映射
假设有以下映射：
maps[0]: serial 设备，地址范围 [0x3f8, 0x3ff]
当访问地址 0x3f8 时, 返回 0（maps[0]）
当访问地址 0x3fc 时, 返回 -1（没有找到对应的映射）
*/
static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
  int i;
  for (i = 0; i < size; i ++) {
    if (map_inside(maps + i, addr)) {
      // 设备的行为无法保持一致，跳过ref的检查，直接同步状态
      difftest_skip_ref();
      return i;
    }
  }
  return -1;
}

// const char *name：设备名称, ioaddr_t addr / paddr_t addr：映射的起始地址
// void *space：设备内存空间指针, uint32_t len：映射区域的长度（字节数）
// io_callback_t callback：访问该区域时触发的回调函数
void add_pio_map(const char *name, ioaddr_t addr,
        void *space, uint32_t len, io_callback_t callback);
void add_mmio_map(const char *name, paddr_t addr,
        void *space, uint32_t len, io_callback_t callback);

word_t map_read(paddr_t addr, int len, IOMap *map);
void map_write(paddr_t addr, int len, word_t data, IOMap *map);

#endif
