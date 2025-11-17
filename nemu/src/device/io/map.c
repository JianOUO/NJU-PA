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

#include <isa.h>
#include <memory/host.h>
#include <memory/vaddr.h>
#include <device/map.h>

// 最大I/O空间为32MB
#define IO_SPACE_MAX (32 * 1024 * 1024)

// 指向I/O空间的起始地址, 在init_map()中初始化
static uint8_t *io_space = NULL;
// 指向已分配的I/O空间的末尾, 在init_map()中初始化
static uint8_t *p_space = NULL;

uint8_t* new_space(int size) {
  // 保存当前分配位置, 作为返回的起始地址
  uint8_t *p = p_space;
  // page aligned, 页对齐(向上取整)
  size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
  // 更新分配指针
  p_space += size;
  // 边界检查, 确保总分配量不超过最大 I/O 空间
  assert(p_space - io_space < IO_SPACE_MAX);
  return p;
}

// 检查访问地址是否在设备的映射区域内
static void check_bound(IOMap *map, paddr_t addr) {
  if (map == NULL) {
    Assert(map != NULL, "address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
  } else {
    Assert(addr <= map->high && addr >= map->low,
        "address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, map->name, map->low, map->high, cpu.pc);
  }
}

// 为什么需要这个包装函数？
// 空指针检查,不是所有设备都需要回调
// 提供统一的调用方式
static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
  if (c != NULL) { c(offset, len, is_write); }
}

// 初始化设备 I/O 内存分配器，为所有设备分配统一的内存池。
void init_map() {
  // "程序生命周期内存"(program-lifetime memory)
  // 程序退出时操作系统会回收所有内存
  io_space = malloc(IO_SPACE_MAX);
  assert(io_space);
  p_space = io_space;
}

word_t map_read(paddr_t addr, int len, IOMap *map) {
  // 长度必须在1-8字节之间
  assert(len >= 1 && len <= 8);
  // 检查访问地址是否在映射区域内
  check_bound(map, addr);
  // 计算相对于映射区域的偏移量
  paddr_t offset = addr - map->low;
  // 先调用回调，用于准备读取数据
  invoke_callback(map->callback, offset, len, false); // prepare data to read
  word_t ret = host_read(map->space + offset, len);
  return ret;
}

void map_write(paddr_t addr, int len, word_t data, IOMap *map) {
  // 长度必须在1-8字节之间
  assert(len >= 1 && len <= 8);
  // 检查访问地址是否在映射区域内
  check_bound(map, addr);
  // 计算相对于映射区域的偏移量
  paddr_t offset = addr - map->low;
  host_write(map->space + offset, len, data);
  // 后调用回调，用于处理写入后的设备逻辑
  invoke_callback(map->callback, offset, len, true);
}
