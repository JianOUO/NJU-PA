#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

// 32KB
#define STACK_SIZE (8 * PGSIZE)

// 使用 union 让同一块内存有两种视角：
typedef union {
  // 视角 1：栈数组
  // PG_ALIGN：页对齐(__attribute((aligned(4096))))，保证起始地址是页边界
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  // 视角 2：进程元数据结构
  struct {
    // 指向保存 CPU 寄存器状态的上下文结构
    Context *cp;
    // 地址空间，包含页大小，用户内存区域信息和页表指针
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    // 记录堆的最大地址，用于判断是否需要调用 _map() 扩展内存映射
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;

#endif
